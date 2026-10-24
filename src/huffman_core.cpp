#include "huffman_core.h"
#include <QCryptographicHash>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <stdexcept>

namespace liss {
namespace {
const QByteArray magic = QByteArray::fromHex("4c4846320d0a1a0a");
constexpr qint64 chunkSize = 64 * 1024;
[[noreturn]] void fail(const QString &message) {
    throw std::runtime_error(message.toUtf8().constData());
}
void ensure(bool condition, const QString &message) {
    if (!condition)
        fail(message);
}
void cancelled(WorkControl *control) {
    if (control && control->cancel.load())
        fail(QStringLiteral("操作已取消；未覆盖目标文件"));
}
void progress(WorkControl *control, int phase, quint64 done, quint64 total) {
    cancelled(control);
    if (control) {
        control->phase.store(phase);
        control->progress.store(total ? static_cast<int>(std::min(100.0L, (100.0L * done) / total)) : 100);
    }
}
void differentFiles(const QString &input, const QString &output) {
    const auto a = QFileInfo(input);
    const auto b = QFileInfo(output);
    ensure(a.isFile(), QStringLiteral("请选择存在的普通文件"));
    QString left = a.canonicalFilePath(), right = b.exists() ? b.canonicalFilePath() : b.absoluteFilePath();
    ensure(left.compare(right, Qt::CaseInsensitive) != 0, QStringLiteral("输入和输出必须是不同文件"));
}
struct Node {
    quint64 frequency;
    int symbol, left = -1, right = -1, smallest;
};
struct Tree {
    std::vector<Node> nodes;
    int root = -1;
    std::array<std::vector<bool>, 256> codes;
};
Tree buildTree(const std::array<quint64, 256> &frequency) {
    Tree tree;
    auto less = [&](int a, int b) {
        const Node &x = tree.nodes[a];
        const Node &y = tree.nodes[b];
        return x.frequency != y.frequency ? x.frequency > y.frequency : x.smallest > y.smallest;
    };
    std::priority_queue<int, std::vector<int>, decltype(less)> queue(less);
    for (int i = 0; i < 256; ++i)
        if (frequency[i]) {
            tree.nodes.push_back({frequency[i], i, -1, -1, i});
            queue.push(static_cast<int>(tree.nodes.size()) - 1);
        }
    while (queue.size() > 1) {
        const int a = queue.top();
        queue.pop();
        const int b = queue.top();
        queue.pop();
        ensure(tree.nodes[a].frequency <= std::numeric_limits<quint64>::max() - tree.nodes[b].frequency,
               QStringLiteral("频率长度溢出"));
        tree.nodes.push_back({tree.nodes[a].frequency + tree.nodes[b].frequency, -1, a, b,
                              std::min(tree.nodes[a].smallest, tree.nodes[b].smallest)});
        queue.push(static_cast<int>(tree.nodes.size()) - 1);
    }
    if (queue.empty())
        return tree;
    tree.root = queue.top();
    std::vector<bool> path;
    std::function<void(int)> visit = [&](int index) {
        const Node &node = tree.nodes[index];
        if (node.symbol >= 0) {
            tree.codes[node.symbol] = path.empty() ? std::vector<bool>{false} : path;
            return;
        }
        path.push_back(false);
        visit(node.left);
        path.back() = true;
        visit(node.right);
        path.pop_back();
    };
    visit(tree.root);
    return tree;
}
void addMetrics(HuffmanStats &stats, const Tree &tree) {
    for (int i = 0; i < 256; ++i) {
        for (bool bit : tree.codes[i])
            stats.codes[i] += bit ? QChar(u'1') : QChar(u'0');
        if (!stats.frequency[i])
            continue;
        const quint64 length = tree.codes[i].size();
        ensure(length &&
                   stats.frequency[i] <= (std::numeric_limits<quint64>::max() - stats.payloadBits) / length,
               QStringLiteral("编码长度溢出"));
        stats.payloadBits += stats.frequency[i] * length;
        const double p = static_cast<double>(stats.frequency[i]) / static_cast<double>(stats.inputBytes);
        stats.entropy -= p * std::log2(p);
        stats.meanBits += p * static_cast<double>(length);
    }
}
class BitWriter {
  public:
    explicit BitWriter(QSaveFile &file) : file_(file) {
        buffer_.reserve(chunkSize);
    }
    void put(bool bit) {
        byte_ = static_cast<quint8>((byte_ << 1) | (bit ? 1 : 0));
        if (++bits_ == 8) {
            append(byte_);
            bits_ = 0;
            byte_ = 0;
        }
    }
    void finish() {
        if (bits_)
            append(static_cast<quint8>(byte_ << (8 - bits_)));
        flush();
    }

  private:
    void append(quint8 value) {
        buffer_.append(static_cast<char>(value));
        if (buffer_.size() >= chunkSize)
            flush();
    }
    void flush() {
        ensure(file_.write(buffer_) == buffer_.size(), QStringLiteral("写入失败；请检查剩余空间和权限"));
        buffer_.clear();
    }
    QSaveFile &file_;
    QByteArray buffer_;
    quint8 byte_ = 0;
    int bits_ = 0;
};
class BitReader {
  public:
    BitReader(QFile &file, quint64 length) : file_(file), length_(length) {}
    bool get() {
        ensure(read_ < length_, QStringLiteral("编码比特不足"));
        if (remaining_ == 0) {
            if (position_ >= buffer_.size()) {
                buffer_ = file_.read(chunkSize);
                position_ = 0;
                ensure(!buffer_.isEmpty(), QStringLiteral("压缩文件被截断"));
            }
            byte_ = static_cast<quint8>(buffer_[position_++]);
            remaining_ = 8;
        }
        --remaining_;
        ++read_;
        return ((byte_ >> remaining_) & 1) != 0;
    }
    void finish() {
        ensure(read_ == length_, QStringLiteral("编码存在多余比特"));
        if (remaining_)
            ensure((byte_ & ((1u << remaining_) - 1u)) == 0, QStringLiteral("末尾填充位损坏"));
    }

  private:
    QFile &file_;
    quint64 length_, read_ = 0;
    QByteArray buffer_;
    qsizetype position_ = 0;
    quint8 byte_ = 0;
    int remaining_ = 0;
};
} // namespace
HuffmanStats HuffmanCodec::compress(const QString &input, const QString &output, WorkControl *control) {
    differentFiles(input, output);
    cancelled(control);
    QFile source(input);
    ensure(source.open(QIODevice::ReadOnly), source.errorString());
    HuffmanStats stats;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    quint64 processed = 0;
    const quint64 sourceSize = static_cast<quint64>(source.size());
    while (!source.atEnd()) {
        const QByteArray block = source.read(chunkSize);
        ensure(!block.isEmpty() || source.error() == QFile::NoError, source.errorString());
        for (unsigned char c : block)
            ++stats.frequency[c];
        hash.addData(block);
        processed += block.size();
        progress(control, 1, processed, sourceSize);
    }
    stats.inputBytes = processed;
    stats.sha256 = hash.result();
    Tree tree = buildTree(stats.frequency);
    addMetrics(stats, tree);
    QSaveFile destination(output);
    ensure(destination.open(QIODevice::WriteOnly), destination.errorString());
    QDataStream stream(&destination);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(magic.constData(), magic.size());
    stream << stats.inputBytes << stats.payloadBits;
    stream.writeRawData(stats.sha256.constData(), stats.sha256.size());
    quint16 symbols = 0;
    for (quint64 f : stats.frequency)
        if (f)
            ++symbols;
    stream << symbols;
    for (int i = 0; i < 256; ++i)
        if (stats.frequency[i])
            stream << static_cast<quint8>(i) << stats.frequency[i];
    ensure(stream.status() == QDataStream::Ok, QStringLiteral("无法写入压缩头"));
    ensure(source.seek(0), QStringLiteral("输入文件不支持重读"));
    BitWriter writer(destination);
    hash.reset();
    processed = 0;
    std::array<quint64, 256> secondFrequency{};
    while (!source.atEnd()) {
        QByteArray block = source.read(chunkSize);
        ensure(!block.isEmpty() || source.error() == QFile::NoError, source.errorString());
        for (unsigned char c : block) {
            ++secondFrequency[c];
            for (bool bit : tree.codes[c])
                writer.put(bit);
        }
        hash.addData(block);
        processed += block.size();
        progress(control, 2, processed, stats.inputBytes);
    }
    ensure(processed == stats.inputBytes && secondFrequency == stats.frequency &&
               hash.result() == stats.sha256,
           QStringLiteral("输入文件在压缩过程中发生变化，请重试"));
    writer.finish();
    stats.outputBytes = destination.pos();
    cancelled(control);
    ensure(destination.commit(), destination.errorString());
    progress(control, 4, 1, 1);
    return stats;
}
HuffmanStats HuffmanCodec::decompress(const QString &input, const QString &output, WorkControl *control) {
    differentFiles(input, output);
    cancelled(control);
    QFile source(input);
    ensure(source.open(QIODevice::ReadOnly), source.errorString());
    QDataStream stream(&source);
    stream.setByteOrder(QDataStream::LittleEndian);
    QByteArray fileMagic(8, '\0');
    ensure(stream.readRawData(fileMagic.data(), 8) == 8 && fileMagic == magic,
           QStringLiteral("不是受支持的 LHF2 文件"));
    HuffmanStats stats;
    quint64 storedBits;
    stream >> stats.inputBytes >> storedBits;
    stats.sha256.resize(32);
    ensure(stream.readRawData(stats.sha256.data(), 32) == 32, QStringLiteral("压缩头被截断"));
    quint16 count = 0;
    stream >> count;
    ensure(count <= 256, QStringLiteral("字符表长度非法"));
    quint64 sum = 0;
    for (quint16 i = 0; i < count; ++i) {
        quint8 symbol = 0;
        quint64 frequency = 0;
        stream >> symbol >> frequency;
        ensure(stream.status() == QDataStream::Ok && frequency && !stats.frequency[symbol],
               QStringLiteral("字符表被截断或含重复项"));
        ensure(frequency <= std::numeric_limits<quint64>::max() - sum, QStringLiteral("频率总数溢出"));
        stats.frequency[symbol] = frequency;
        sum += frequency;
    }
    ensure(stream.status() == QDataStream::Ok && sum == stats.inputBytes,
           QStringLiteral("原始长度与字符频率不一致"));
    ensure(stats.inputBytes <= static_cast<quint64>(std::numeric_limits<qint64>::max()),
           QStringLiteral("文件长度超出平台范围"));
    Tree tree = buildTree(stats.frequency);
    addMetrics(stats, tree);
    ensure(stats.payloadBits == storedBits, QStringLiteral("编码长度与字符表不一致"));
    const quint64 payloadBytes = storedBits / 8 + (storedBits % 8 != 0);
    ensure(payloadBytes == static_cast<quint64>(source.size() - source.pos()),
           QStringLiteral("压缩文件长度不匹配或存在多余数据"));
    QSaveFile destination(output);
    ensure(destination.open(QIODevice::WriteOnly), destination.errorString());
    BitReader reader(source, storedBits);
    QByteArray block;
    block.reserve(chunkSize);
    QCryptographicHash hash(QCryptographicHash::Sha256);
    std::array<quint64, 256> actual{};
    auto flush = [&]() {
        ensure(destination.write(block) == block.size(),
               QStringLiteral("解压写入失败；请检查剩余空间和权限"));
        hash.addData(block);
        block.clear();
    };
    for (quint64 done = 0; done < stats.inputBytes; ++done) {
        int current = tree.root;
        ensure(current >= 0, QStringLiteral("缺少编码树"));
        if (tree.nodes[current].symbol >= 0)
            ensure(!reader.get(), QStringLiteral("单字符编码损坏"));
        else
            while (tree.nodes[current].symbol < 0)
                current = reader.get() ? tree.nodes[current].right : tree.nodes[current].left;
        const int symbol = tree.nodes[current].symbol;
        ++actual[symbol];
        block.append(static_cast<char>(symbol));
        if (block.size() >= chunkSize) {
            flush();
            progress(control, 3, done + 1, stats.inputBytes);
        }
    }
    flush();
    reader.finish();
    ensure(actual == stats.frequency && hash.result() == stats.sha256,
           QStringLiteral("SHA-256 完整性校验失败，文件可能损坏"));
    stats.outputBytes = destination.pos();
    cancelled(control);
    ensure(destination.commit(), destination.errorString());
    progress(control, 4, 1, 1);
    return stats;
}
QString humanBytes(quint64 bytes) {
    const QStringList units{QStringLiteral("B"), QStringLiteral("KiB"), QStringLiteral("MiB"),
                            QStringLiteral("GiB"), QStringLiteral("TiB")};
    double value = static_cast<double>(bytes);
    int i = 0;
    while (value >= 1024 && i < units.size() - 1) {
        value /= 1024;
        ++i;
    }
    return QStringLiteral("%1 %2").arg(QString::number(value, 'f', i ? 1 : 0), units[i]);
}
} // namespace liss
