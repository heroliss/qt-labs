#include "calc_core.h"
#include "huffman_core.h"
#include "os_core.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTextStream>
#include <cmath>
#include <functional>
#include <numbers>
#include <random>
#include <stdexcept>

using namespace liss;
namespace {
int passed = 0, failed = 0;
QJsonArray checks;
void require(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
template <class F> void throws(F action) {
    bool caught = false;
    try {
        action();
    } catch (const std::exception &) {
        caught = true;
    }
    require(caught, "Expected rejection");
}
void check(const QString &name, const std::function<void()> &test) {
    try {
        test();
        ++passed;
        checks.append(QJsonObject{{QStringLiteral("name"), name}, {QStringLiteral("passed"), true}});
    } catch (const std::exception &e) {
        ++failed;
        checks.append(QJsonObject{{QStringLiteral("name"), name},
                                  {QStringLiteral("passed"), false},
                                  {QStringLiteral("error"), QString::fromUtf8(e.what())}});
    }
}
void write(const QString &path, const QByteArray &bytes) {
    QFile file(path);
    require(file.open(QIODevice::WriteOnly), "Write open failed");
    require(file.write(bytes) == bytes.size(), "Write failed");
}
QByteArray read(const QString &path) {
    QFile file(path);
    require(file.open(QIODevice::ReadOnly), "Read open failed");
    return file.readAll();
}
void calc(const QString &expression, double expected, const CalcContext &context = {}) {
    const double actual = static_cast<double>(evaluate(expression, context).value);
    require(std::abs(actual - expected) <= 1e-10 * std::max(1., std::abs(expected)),
            "Wrong calculator result");
}
} // namespace
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    check(QStringLiteral("OS validates creation, scheduling, blocking, wake and termination"), [] {
        OsModel m(32);
        int a = m.create(QStringLiteral("A"), 8, 10), b = m.create(QStringLiteral("B"), 9, 12);
        require(m.running() == a, "First process must run");
        m.step(3);
        require(m.running() == b, "RR must rotate");
        m.block();
        require(m.blocked().front() == b && m.running() == a, "Blocking must dispatch");
        require(m.wake(b), "Wake failed");
        require(!m.wake(b), "Double wake accepted");
        m.terminate(a);
        m.terminate(b);
        require(m.freeBlocks() == 32 && m.validate(), "Memory leak after termination");
    });
    check(QStringLiteral("OS rejects invalid inputs and empty operations safely"), [] {
        OsModel m(16);
        throws([&] { m.create(QString(), 1, 1); });
        throws([&] { m.create(QStringLiteral("A"), 0, 1); });
        throws([&] { m.create(QStringLiteral("A"), 1, 0); });
        throws([&] { m.create(QStringLiteral("A"), 17, 1); });
        throws([&] { m.step(0); });
        require(!m.step() && !m.block() && !m.wake(5) && !m.terminate(7), "Empty operations unsafe");
        require(m.validate(), "Invalid empty model");
    });
    check(QStringLiteral("OS worst-fit selects largest free region"), [] {
        OsModel m(32);
        m.create(QStringLiteral("A"), 5, 10);
        m.create(QStringLiteral("B"), 5, 10);
        m.create(QStringLiteral("C"), 5, 10);
        m.terminate(2);
        int d = m.create(QStringLiteral("D"), 4, 10);
        require(m.processes().at(d).start == 15, "Worst-fit not preserved");
        require(m.validate(), "Invalid worst-fit model");
    });
    check(QStringLiteral("OS fragmented allocation requires or performs compaction"), [] {
        OsModel m(32);
        for (int i = 0; i < 4; ++i)
            m.create(QString::number(i), 8, 10);
        m.terminate(2);
        m.terminate(4);
        require(m.freeBlocks() == 16 && m.largestFree() == 8, "Fragment setup failed");
        throws([&] { m.create(QStringLiteral("X"), 12, 10, false); });
        int id = m.create(QStringLiteral("X"), 12, 10, true);
        require(m.processes().at(id).start == 16 && m.compactions() == 1 && m.validate(),
                "Auto compaction failed");
    });
    check(QStringLiteral("OS compaction preserves queues, IDs and CPU state"), [] {
        OsModel m;
        m.showcase();
        auto ready = m.ready(), blocked = m.blocked();
        auto running = m.running();
        int free = m.freeBlocks();
        m.compact();
        require(ready == m.ready() && blocked == m.blocked() && running == m.running() &&
                    free == m.freeBlocks() && m.validate(),
                "Compaction changed semantic state");
        require(m.freeRegions().size() == 1, "Compaction left holes");
    });
    check(QStringLiteral("OS completes CPU bursts without stale state"), [] {
        OsModel m;
        for (int i = 0; i < 10; ++i)
            m.create(QString::number(i), 3, i + 1);
        for (int i = 0; i < 100; ++i) {
            m.step(3);
            require(m.validate(), "Invalid completion state");
        }
        require(m.completed() == 10 && m.processes().empty() && m.elapsed() == 55 && m.freeBlocks() == 128,
                "Burst accounting failed");
    });
    check(QStringLiteral("OS 10000 randomized transitions conserve memory and queue membership"), [] {
        OsModel m(128);
        std::mt19937 rng(20261024);
        for (int i = 0; i < 10000; ++i) {
            try {
                switch (rng() % 7) {
                case 0:
                    m.create(QStringLiteral("worker"), 1 + rng() % 32, 1 + rng() % 100, rng() % 2);
                    break;
                case 1:
                    m.step(1 + rng() % 8);
                    break;
                case 2:
                    m.block();
                    break;
                case 3:
                    if (!m.blocked().empty())
                        m.wake(m.blocked().front());
                    break;
                case 4:
                    if (!m.processes().empty()) {
                        auto it = m.processes().begin();
                        std::advance(it, rng() % m.processes().size());
                        m.terminate(it->first);
                    }
                    break;
                case 5:
                    m.compact();
                    break;
                case 6:
                    m.wake(-1);
                    break;
                }
            } catch (const std::runtime_error &) {
            }
            require(m.validate(), "Randomized invariant failed");
        }
    });
    QTemporaryDir dir;
    require(dir.isValid(), "No test temp directory");
    const QString input = dir.filePath(QStringLiteral("输入 原始.bin")),
                  encoded = dir.filePath(QStringLiteral("压缩.lhf")),
                  output = dir.filePath(QStringLiteral("恢复.bin"));
    std::vector<std::pair<QString, QByteArray>> datasets{
        {QStringLiteral("empty"), {}},
        {QStringLiteral("one byte"), QByteArray(1, 'x')},
        {QStringLiteral("single symbol"), QByteArray(100000, '\0')},
        {QStringLiteral("UTF-8 text"), QStringLiteral("哈夫曼编码 · 无损恢复\n").repeated(700).toUtf8()}};
    QByteArray all;
    for (int i = 0; i < 256; ++i)
        all.append(static_cast<char>(i));
    datasets.push_back({QStringLiteral("all 256 symbols"), all.repeated(400)});
    std::mt19937 rng(7231);
    QByteArray random(1000001, '\0');
    for (char &c : random)
        c = static_cast<char>(rng());
    datasets.push_back({QStringLiteral("random binary"), random});
    datasets.push_back(
        {QStringLiteral("multi-chunk telemetry"),
         QByteArrayLiteral("[INFO] renderer=ready asset=terrain load=complete\n").repeated(80000)});
    for (const auto &[name, bytes] : datasets)
        check(QStringLiteral("Huffman roundtrip: ") + name, [&] {
            write(input, bytes);
            auto stats = HuffmanCodec::compress(input, encoded);
            auto restored = HuffmanCodec::decompress(encoded, output);
            require(read(output) == bytes, "Decoded bytes differ");
            require(stats.inputBytes == static_cast<quint64>(bytes.size()) &&
                        restored.inputBytes == stats.inputBytes,
                    "Incorrect size");
            const QByteArray first = read(encoded);
            HuffmanCodec::compress(input, encoded);
            require(read(encoded) == first, "Encoding not deterministic");
        });
    write(input, QByteArrayLiteral("ABRACADABRA ").repeated(1000));
    HuffmanCodec::compress(input, encoded);
    const QByteArray valid = read(encoded);
    check(QStringLiteral("Huffman rejects same source and destination"), [&] {
        throws([&] { HuffmanCodec::compress(input, input); });
        throws([&] { HuffmanCodec::decompress(encoded, encoded); });
    });
    check(QStringLiteral("Huffman rejects truncation and preserves existing output"), [&] {
        write(encoded, valid.left(valid.size() - 1));
        write(output, QByteArrayLiteral("KEEP"));
        throws([&] { HuffmanCodec::decompress(encoded, output); });
        require(read(output) == QByteArrayLiteral("KEEP"), "Truncation replaced destination");
    });
    check(QStringLiteral("Huffman rejects trailing bytes"), [&] {
        write(encoded, valid + QByteArrayLiteral("X"));
        throws([&] { HuffmanCodec::decompress(encoded, output); });
    });
    check(QStringLiteral("Huffman detects SHA-256 corruption and retains destination"), [&] {
        QByteArray bad = valid;
        bad[25] ^= 1;
        write(encoded, bad);
        write(output, QByteArrayLiteral("KEEP"));
        throws([&] { HuffmanCodec::decompress(encoded, output); });
        require(read(output) == QByteArrayLiteral("KEEP"), "Corruption replaced destination");
    });
    check(QStringLiteral("Huffman rejects corrupt payload"), [&] {
        QByteArray bad = valid;
        bad[bad.size() - 3] ^= 0x20;
        write(encoded, bad);
        throws([&] { HuffmanCodec::decompress(encoded, output); });
    });
    check(QStringLiteral("Huffman cancellation leaves source and output intact"), [&] {
        write(encoded, valid);
        write(output, QByteArrayLiteral("KEEP"));
        WorkControl control;
        control.cancel = true;
        throws([&] { HuffmanCodec::compress(input, output, &control); });
        throws([&] { HuffmanCodec::decompress(encoded, output, &control); });
        require(read(output) == QByteArrayLiteral("KEEP") && read(encoded) == valid,
                "Cancellation changed files");
    });
    check(QStringLiteral("Huffman rejects invalid magic and impossible header length"), [&] {
        QByteArray bad = valid;
        bad[0] = 'X';
        write(encoded, bad);
        throws([&] { HuffmanCodec::decompress(encoded, output); });
        bad = valid;
        for (int i = 8; i < 16; ++i)
            bad[i] = static_cast<char>(0xff);
        write(encoded, bad);
        throws([&] { HuffmanCodec::decompress(encoded, output); });
    });
    const std::vector<std::pair<QString, double>> cases{{QStringLiteral("1+2*3"), 7},
                                                        {QStringLiteral("(1+2)*3"), 9},
                                                        {QStringLiteral("2^3^2"), 512},
                                                        {QStringLiteral("-2^2"), -4},
                                                        {QStringLiteral("(-2)^2"), 4},
                                                        {QStringLiteral("2^-3"), .125},
                                                        {QStringLiteral("2^-3^2"), 1. / 512},
                                                        {QStringLiteral("--2"), 2},
                                                        {QStringLiteral(".5+1.25"), 1.75},
                                                        {QStringLiteral("1e-3+2E2"), 200.001},
                                                        {QStringLiteral("2pi"), 2 * std::numbers::pi},
                                                        {QStringLiteral("2(3+4)"), 14},
                                                        {QStringLiteral("(2+1)(4-1)"), 9},
                                                        {QStringLiteral("sqrt(0)"), 0},
                                                        {QStringLiteral("sqrt(3^2+4^2)+sin(30)*12"), 11},
                                                        {QStringLiteral("[-3]+abs(-4)"), 7},
                                                        {QStringLiteral("sin(30)^2+cos(30)^2"), 1},
                                                        {QStringLiteral("asin(0.5)"), 30},
                                                        {QStringLiteral("acos(0)"), 90},
                                                        {QStringLiteral("atan(1)"), 45},
                                                        {QStringLiteral("ln(exp(2))"), 2},
                                                        {QStringLiteral("lg(1000)"), 3},
                                                        {QStringLiteral("ceil(1.2)+floor(1.8)"), 3},
                                                        {QStringLiteral("10%3"), 1},
                                                        {QStringLiteral("sinh(0)+cosh(0)+tanh(0)"), 1},
                                                        {QStringLiteral("2×3−4÷2"), 4}};
    for (const auto &[expression, expected] : cases)
        check(QStringLiteral("Calculator: ") + expression, [&] { calc(expression, expected); });
    check(QStringLiteral("Calculator radians, variables, Ans and memory"), [] {
        CalcContext context;
        context.degrees = false;
        context.variables = {2, 3, 4};
        context.memory = 12;
        context.ans = 6;
        calc(QStringLiteral("sin(pi/2)+X*Y+Z+M+Ans"), 29, context);
        calc(QStringLiteral("asin(1)"), std::numbers::pi / 2, context);
    });
    for (const QString &expression :
         {QString(), QStringLiteral(" "), QStringLiteral("1/0"), QStringLiteral("1%0"),
          QStringLiteral("sqrt(-1)"), QStringLiteral("ln(0)"), QStringLiteral("asin(2)"),
          QStringLiteral("tan(90)"), QStringLiteral("1+"), QStringLiteral("()"), QStringLiteral("(1+2]"),
          QStringLiteral("sin30"), QStringLiteral("foo(2)"), QStringLiteral("1..2"), QStringLiteral("2**3"),
          QStringLiteral("exp(10000)"), QStringLiteral("(-1)^0.5"), QStringLiteral("1e999")})
        check(QStringLiteral("Calculator rejects: ") + expression,
              [&] { throws([&] { evaluate(expression); }); });
    check(QStringLiteral("Calculator input and nesting bounds"), [] {
        throws([] { evaluate(QString(9000, QChar(u'1'))); });
        throws([] { evaluate(QString(300, QChar(u'(')) + QStringLiteral("1") + QString(300, QChar(u')'))); });
    });
    QJsonObject report{{QStringLiteral("passed"), passed},
                       {QStringLiteral("failed"), failed},
                       {QStringLiteral("checks"), checks}};
    QTextStream(stdout) << QJsonDocument(report).toJson() << Qt::endl;
    return failed ? 1 : 0;
}
