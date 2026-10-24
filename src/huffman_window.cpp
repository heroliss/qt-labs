#include "common.h"
#include "huffman_core.h"
#include <QCloseEvent>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>
#include <memory>

namespace liss {
namespace {
QString symbolLabel(int symbol) {
    if (symbol == 10)
        return QStringLiteral("LF");
    if (symbol == 13)
        return QStringLiteral("CR");
    if (symbol == 32)
        return QStringLiteral("SPACE");
    if (symbol >= 33 && symbol <= 126)
        return QString(QChar(symbol));
    return QStringLiteral("0x%1").arg(symbol, 2, 16, QChar(u'0')).toUpper();
}
class FrequencyView : public QWidget {
  public:
    FrequencyView() {
        setMinimumHeight(185);
        setAccessibleName(QStringLiteral("最常见字节频率图"));
    }
    void setStats(const HuffmanStats &value) {
        stats_ = value;
        update();
    }

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        std::vector<int> order;
        for (int i = 0; i < 256; ++i)
            if (stats_.frequency[i])
                order.push_back(i);
        std::sort(order.begin(), order.end(),
                  [this](int a, int b) { return stats_.frequency[a] > stats_.frequency[b]; });
        if (order.empty()) {
            p.setPen(QColor(QStringLiteral("#8fa3bf")));
            p.drawText(rect(), Qt::AlignCenter, QStringLiteral("打开文件并完成一次压缩后，查看频率分布。"));
            return;
        }
        const int count = std::min(7, static_cast<int>(order.size()));
        const qreal rowH = height() / static_cast<qreal>(count), barW = width() - 130.;
        for (int i = 0; i < count; ++i) {
            int symbol = order[i];
            qreal y = i * rowH + 3;
            p.setPen(QColor(QStringLiteral("#b9c9df")));
            p.drawText(QRectF(0, y, 66, rowH - 4), Qt::AlignVCenter, symbolLabel(symbol));
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(QStringLiteral("#20324b")));
            p.drawRoundedRect(QRectF(70, y + 4, barW, rowH - 13), 4, 4);
            p.setBrush(processColor(i + 1));
            p.drawRoundedRect(
                QRectF(70, y + 4,
                       barW * static_cast<double>(stats_.frequency[symbol]) / stats_.frequency[order[0]],
                       rowH - 13),
                4, 4);
            p.setPen(QColor(QStringLiteral("#b8cce4")));
            p.drawText(
                QRectF(width() - 56, y, 56, rowH - 4), Qt::AlignRight | Qt::AlignVCenter,
                QStringLiteral("%1%").arg(100. * stats_.frequency[symbol] / stats_.inputBytes, 0, 'f', 1));
        }
    }

  private:
    HuffmanStats stats_;
};
struct Outcome {
    bool okay = false;
    QString error;
    HuffmanStats stats;
    qint64 ms = 0;
};
class HuffmanWindow : public QMainWindow {
  public:
    HuffmanWindow(bool showcase, const QString &scenario) {
        setWindowTitle(QStringLiteral("Liss Huffman Lab · 字节与编码"));
        resize(1400, 960);
        setMinimumSize(1100, 760);
        setAcceptDrops(true);
        auto *root = new QWidget;
        root->setObjectName(QStringLiteral("root"));
        setCentralWidget(root);
        auto *layout = new QVBoxLayout(root);
        layout->setContentsMargins(24, 22, 24, 18);
        layout->setSpacing(15);
        auto *head = new QHBoxLayout;
        head->addWidget(
            heading(QStringLiteral("LISS QT LABS  /  LOSSLESS COMPRESSION"),
                    QStringLiteral("让每个字节，找到更短的表达"),
                    QStringLiteral("自写 Huffman 编码 · 任意二进制 · 流式处理 · SHA-256 完整性校验")),
            1);
        sample_ = button(QStringLiteral("载入样例文件"), QStringLiteral("loadSample"));
        head->addWidget(sample_);
        layout->addLayout(head);
        auto *metrics = new QHBoxLayout;
        metrics->setSpacing(12);
        metrics->addWidget(
            metricCard(QStringLiteral("原始文件"), &sourceSize_, QStringLiteral("按 64 KiB 数据块读取")));
        metrics->addWidget(metricCard(QStringLiteral("压缩文件"), &compressedSize_,
                                      QStringLiteral("包含频率表与完整性摘要")));
        metrics->addWidget(
            metricCard(QStringLiteral("空间节省"), &ratio_, QStringLiteral("负值表示压缩后体积增大")));
        metrics->addWidget(metricCard(QStringLiteral("平均码长"), &mean_, QStringLiteral("bits / byte")));
        layout->addLayout(metrics);
        auto *body = new QHBoxLayout;
        body->setSpacing(15);
        auto *left = new QVBoxLayout;
        left->setSpacing(14);
        auto *right = new QVBoxLayout;
        right->setSpacing(14);
        QVBoxLayout *fileLayout;
        auto *filePanel =
            panel(QStringLiteral("文件工作区"),
                  QStringLiteral("拖入一个文件，或使用“选择文件”。原文件始终保留。"), &fileLayout);
        fileLayout->addWidget(text(QStringLiteral("输入文件"), QStringLiteral("muted")));
        auto *inputRow = new QHBoxLayout;
        input_ = new QLineEdit;
        input_->setObjectName(QStringLiteral("inputPath"));
        input_->setPlaceholderText(QStringLiteral("选择任意文本、图片或二进制文件"));
        input_->setAccessibleName(QStringLiteral("输入文件路径"));
        chooseInput_ = button(QStringLiteral("选择文件"), QStringLiteral("chooseInput"));
        inputRow->addWidget(input_, 1);
        inputRow->addWidget(chooseInput_);
        fileLayout->addLayout(inputRow);
        inputInfo_ = text(QStringLiteral("尚未选择文件"), QStringLiteral("muted"));
        fileLayout->addWidget(inputInfo_);
        fileLayout->addWidget(text(QStringLiteral("输出文件"), QStringLiteral("muted")));
        auto *outputRow = new QHBoxLayout;
        output_ = new QLineEdit;
        output_->setObjectName(QStringLiteral("outputPath"));
        output_->setAccessibleName(QStringLiteral("输出文件路径"));
        chooseOutput_ = button(QStringLiteral("保存到…"), QStringLiteral("chooseOutput"));
        outputRow->addWidget(output_, 1);
        outputRow->addWidget(chooseOutput_);
        fileLayout->addLayout(outputRow);
        auto *actions = new QHBoxLayout;
        compress_ = button(QStringLiteral("压缩为 .lhf"), QStringLiteral("compressFile"), true);
        decompress_ = button(QStringLiteral("解压 .lhf"), QStringLiteral("decompressFile"));
        actions->addWidget(compress_);
        actions->addWidget(decompress_);
        fileLayout->addLayout(actions);
        progress_ = new QProgressBar;
        progress_->setRange(0, 100);
        progress_->setValue(0);
        fileLayout->addWidget(progress_);
        auto *statusRow = new QHBoxLayout;
        status_ = text(QStringLiteral("就绪"), QStringLiteral("muted"));
        status_->setObjectName(QStringLiteral("operationStatus"));
        cancel_ = button(QStringLiteral("取消"), QStringLiteral("cancelOperation"));
        cancel_->setEnabled(false);
        statusRow->addWidget(status_, 1);
        statusRow->addWidget(cancel_);
        fileLayout->addLayout(statusRow);
        left->addWidget(filePanel);
        QVBoxLayout *verifyLayout;
        auto *verifyPanel =
            panel(QStringLiteral("完整性与信息量"),
                  QStringLiteral("解压通过摘要与字符频率双重校验后，才会提交目标文件。"), &verifyLayout);
        integrity_ = text(QStringLiteral("等待编解码结果"), QStringLiteral("section"));
        verifyLayout->addWidget(integrity_);
        entropy_ = text(QStringLiteral("信息熵  —  ·  不同字节  —"), QStringLiteral("muted"));
        verifyLayout->addWidget(entropy_);
        hash_ = text(QStringLiteral("SHA-256\n—"), QStringLiteral("muted"));
        hash_->setFont(QFont(QStringLiteral("Consolas"), 10));
        hash_->setMinimumHeight(52);
        verifyLayout->addWidget(hash_);
        auto *details = new QTabWidget;
        details->setObjectName(QStringLiteral("fileDetailsTabs"));
        details->addTab(verifyPanel, QStringLiteral("完整性"));
        QVBoxLayout *logLayout;
        auto *logPanel =
            panel(QStringLiteral("操作记录"), QStringLiteral("耗时、文件大小与往返校验"), &logLayout);
        log_ = new QPlainTextEdit;
        log_->setReadOnly(true);
        log_->setObjectName(QStringLiteral("operationLog"));
        log_->setMaximumBlockCount(200);
        log_->setMinimumHeight(90);
        logLayout->addWidget(log_, 1);
        details->addTab(logPanel, QStringLiteral("操作记录"));
        left->addWidget(details, 1);
        QVBoxLayout *freqLayout;
        auto *freqPanel = panel(QStringLiteral("字节频率"),
                                QStringLiteral("出现更频繁的字节，通常获得更短的编码。"), &freqLayout);
        frequency_ = new FrequencyView;
        freqLayout->addWidget(frequency_, 1);
        right->addWidget(freqPanel, 2);
        QVBoxLayout *codesLayout;
        auto *codePanel =
            panel(QStringLiteral("Huffman 编码表"),
                  QStringLiteral("从低频叶子逐步合并；同频时以字节值打破平局，保证可重复。"), &codesLayout);
        codes_ = new QTableWidget;
        codes_->setObjectName(QStringLiteral("codeTable"));
        prepareTable(codes_, {QStringLiteral("字节"), QStringLiteral("频数"), QStringLiteral("码长"),
                              QStringLiteral("前缀编码")});
        codes_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        codes_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        codes_->setMinimumHeight(230);
        codesLayout->addWidget(codes_, 1);
        right->addWidget(codePanel, 3);
        body->addLayout(left, 5);
        body->addLayout(right, 6);
        layout->addLayout(body, 1);
        auto *pipeline = new QHBoxLayout;
        pipeline->setSpacing(12);
        for (const auto &pair : std::vector<std::pair<QString, QString>>{
                 {QStringLiteral("01  统计"), QStringLiteral("256 字节频率 / 原始摘要")},
                 {QStringLiteral("02  构树"), QStringLiteral("最小堆合并 / 无歧义前缀码")},
                 {QStringLiteral("03  输出"), QStringLiteral("比特打包 / 原子保存 / 校验恢复")}}) {
            QVBoxLayout *item;
            pipeline->addWidget(panel(pair.first, pair.second, &item));
        }
        layout->addLayout(pipeline);
        connect(chooseInput_, &QPushButton::clicked, this, [this] {
            QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择待处理文件"));
            if (!path.isEmpty())
                setInput(path);
        });
        connect(input_, &QLineEdit::editingFinished, this, [this] { setInput(input_->text()); });
        connect(chooseOutput_, &QPushButton::clicked, this, [this] {
            QString path =
                QFileDialog::getSaveFileName(this, QStringLiteral("选择输出位置"), output_->text());
            if (!path.isEmpty())
                output_->setText(path);
        });
        connect(compress_, &QPushButton::clicked, this, [this] { start(false); });
        connect(decompress_, &QPushButton::clicked, this, [this] { start(true); });
        connect(cancel_, &QPushButton::clicked, this, [this] {
            if (control_)
                control_->cancel.store(true);
            cancel_->setEnabled(false);
            status_->setText(QStringLiteral("正在取消…"));
        });
        connect(sample_, &QPushButton::clicked, this, [this] { loadSample(); });
        connect(&watcher_, &QFutureWatcher<Outcome>::finished, this, [this] {
            Outcome outcome = watcher_.result();
            poll_.stop();
            setBusy(false);
            if (outcome.okay) {
                showStats(outcome.stats);
                progress_->setValue(100);
                status_->setText(QStringLiteral("完成 · %1 ms · 已保存结果").arg(outcome.ms));
                append(QStringLiteral("%1完成 · %2 ms\n%3")
                           .arg(wasDecode_ ? QStringLiteral("解压") : QStringLiteral("压缩"))
                           .arg(outcome.ms)
                           .arg(output_->text()));
                if (wasDecode_)
                    integrity_->setText(QStringLiteral("SHA-256 校验通过 · 文件完整"));
            } else {
                status_->setText(outcome.error);
                append(outcome.error);
            }
        });
        connect(&poll_, &QTimer::timeout, this, [this] {
            if (!control_)
                return;
            progress_->setValue(control_->progress.load());
            QStringList phases{QStringLiteral("准备"), QStringLiteral("统计频率与摘要"),
                               QStringLiteral("写入压缩比特流"), QStringLiteral("解码与校验"),
                               QStringLiteral("已完成")};
            if (!control_->cancel.load())
                status_->setText(phases.value(control_->phase.load()));
        });
        if (showcase) {
            loadSample();
            if (scenario == QStringLiteral("decoded"))
                integrity_->setText(QStringLiteral("SHA-256 校验通过 · 恢复内容一致"));
        }
    }
    ~HuffmanWindow() override {
        if (control_)
            control_->cancel.store(true);
        watcher_.waitForFinished();
    }

  protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (!watcher_.isRunning() && event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1 &&
            event->mimeData()->urls().first().isLocalFile())
            event->acceptProposedAction();
    }
    void dropEvent(QDropEvent *event) override {
        if (!watcher_.isRunning() && event->mimeData()->hasUrls()) {
            QString path = event->mimeData()->urls().first().toLocalFile();
            if (QFileInfo(path).isFile()) {
                setInput(path);
                event->acceptProposedAction();
            }
        }
    }

  private:
    void append(const QString &message) {
        log_->appendPlainText(QStringLiteral("[%1] %2").arg(
            QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), message));
    }
    void setInput(const QString &path) {
        input_->setText(path);
        input_->setCursorPosition(0);
        QFileInfo info(path);
        inputInfo_->setText(info.isFile()
                                ? QStringLiteral("%1 · %2").arg(info.fileName(), humanBytes(info.size()))
                                : QStringLiteral("请选择存在的普通文件"));
        output_->setText(info.suffix().compare(QStringLiteral("lhf"), Qt::CaseInsensitive) == 0
                             ? info.absolutePath() + QChar(u'/') + info.completeBaseName() +
                                   QStringLiteral(".restored")
                             : path + QStringLiteral(".lhf"));
    }
    void setBusy(bool busy) {
        for (QWidget *widget : std::vector<QWidget *>{input_, output_, chooseInput_, chooseOutput_, compress_,
                                                      decompress_, sample_})
            widget->setEnabled(!busy);
        cancel_->setEnabled(busy);
    }
    void start(bool decode) {
        if (watcher_.isRunning())
            return;
        const QString source = input_->text().trimmed(), destination = output_->text().trimmed();
        if (!QFileInfo(source).isFile() || destination.isEmpty()) {
            status_->setText(QStringLiteral("请先选择输入文件和输出位置。"));
            return;
        }
        if (QFileInfo::exists(destination) &&
            QMessageBox::question(
                this, QStringLiteral("覆盖目标文件？"),
                QStringLiteral("%1\n\n完成且校验成功后，替换这个文件？").arg(destination)) !=
                QMessageBox::Yes)
            return;
        wasDecode_ = decode;
        control_ = std::make_shared<WorkControl>();
        auto control = control_;
        setBusy(true);
        sourceSize_->setText(decode ? QStringLiteral("—") : humanBytes(QFileInfo(source).size()));
        compressedSize_->setText(QStringLiteral("—"));
        ratio_->setText(QStringLiteral("—"));
        mean_->setText(QStringLiteral("—"));
        integrity_->setText(QStringLiteral("正在处理当前文件…"));
        entropy_->setText(QStringLiteral("统计完成后显示信息量与编码表"));
        hash_->setText(QStringLiteral("SHA-256\n—"));
        frequency_->setStats(HuffmanStats{});
        codes_->setRowCount(0);
        progress_->setValue(0);
        status_->setText(QStringLiteral("正在启动…"));
        append(QStringLiteral("%1 · %2").arg(decode ? QStringLiteral("解压") : QStringLiteral("压缩"),
                                             QFileInfo(source).fileName()));
        watcher_.setFuture(QtConcurrent::run([source, destination, control, decode] {
            Outcome outcome;
            QElapsedTimer timer;
            timer.start();
            try {
                outcome.stats = decode ? HuffmanCodec::decompress(source, destination, control.get())
                                       : HuffmanCodec::compress(source, destination, control.get());
                outcome.okay = true;
            } catch (const std::exception &error) {
                outcome.error = QString::fromUtf8(error.what());
            }
            outcome.ms = timer.elapsed();
            return outcome;
        }));
        poll_.start(80);
    }
    void showStats(const HuffmanStats &stats) {
        sourceSize_->setText(humanBytes(stats.inputBytes));
        const quint64 compressed = 58 +
                                   9 * std::count_if(stats.frequency.begin(), stats.frequency.end(),
                                                     [](quint64 value) { return value != 0; }) +
                                   stats.payloadBits / 8 + (stats.payloadBits % 8 != 0);
        compressedSize_->setText(humanBytes(compressed));
        double saving =
            stats.inputBytes ? 100. * (1. - static_cast<double>(compressed) / stats.inputBytes) : 0.;
        if (std::abs(saving) < 0.05)
            saving = 0.;
        ratio_->setText(stats.inputBytes ? QStringLiteral("%1 %").arg(saving, 0, 'f', 1)
                                         : QStringLiteral("—"));
        mean_->setText(QStringLiteral("%1").arg(stats.meanBits, 0, 'f', 3));
        entropy_->setText(QStringLiteral("信息熵 %1 bits/byte  ·  不同字节 %2")
                              .arg(stats.entropy, 0, 'f', 3)
                              .arg(std::count_if(stats.frequency.begin(), stats.frequency.end(),
                                                 [](quint64 v) { return v != 0; })));
        integrity_->setText(QStringLiteral("原始文件摘要已记录"));
        const QString hex = QString::fromLatin1(stats.sha256.toHex());
        hash_->setText(QStringLiteral("SHA-256\n%1\n%2").arg(hex.left(32), hex.mid(32)));
        frequency_->setStats(stats);
        std::vector<int> symbols;
        for (int i = 0; i < 256; ++i)
            if (stats.frequency[i])
                symbols.push_back(i);
        std::sort(symbols.begin(), symbols.end(),
                  [&](int a, int b) { return stats.frequency[a] > stats.frequency[b]; });
        codes_->setRowCount(static_cast<int>(symbols.size()));
        int row = 0;
        for (int symbol : symbols) {
            setCell(codes_, row, 0, symbolLabel(symbol), processColor(row % 8 + 1));
            setCell(codes_, row, 1, QString::number(stats.frequency[symbol]));
            setCell(codes_, row, 2, QString::number(stats.codes[symbol].size()));
            setCell(codes_, row, 3, stats.codes[symbol]);
            codes_->item(row, 3)->setFont(QFont(QStringLiteral("Consolas"), 10));
            ++row;
        }
    }
    void loadSample() {
        if (watcher_.isRunning())
            return;
        if (!sampleDir_ || !sampleDir_->isValid())
            sampleDir_ = std::make_unique<QTemporaryDir>();
        if (!sampleDir_->isValid()) {
            status_->setText(QStringLiteral("无法创建样例目录"));
            return;
        }
        const QString source = sampleDir_->filePath(QStringLiteral("telemetry-sample.log")),
                      packed = source + QStringLiteral(".lhf"),
                      restored = source + QStringLiteral(".restored");
        QFile file(source);
        if (!file.open(QIODevice::WriteOnly)) {
            status_->setText(file.errorString());
            return;
        }
        const QByteArray line =
            QByteArrayLiteral("[INFO] frame=0016 renderer=ready asset=terrain load=complete\n[INFO] "
                              "frame=0017 renderer=ready asset=terrain load=complete\n");
        for (int i = 0; i < 4096; ++i)
            file.write(line);
        file.close();
        try {
            QElapsedTimer timer;
            timer.start();
            auto stats = HuffmanCodec::compress(source, packed);
            HuffmanCodec::decompress(packed, restored);
            setInput(source);
            showStats(stats);
            progress_->setValue(100);
            integrity_->setText(QStringLiteral("压缩 → 解压 → SHA-256 校验通过"));
            status_->setText(QStringLiteral("样例往返完成 · %1 ms").arg(timer.elapsed()));
            log_->clear();
            append(QStringLiteral("生成样例 telemetry-sample.log · %1").arg(humanBytes(stats.inputBytes)));
            append(QStringLiteral("编码完成 · %1 · 平均 %2 bits/byte")
                       .arg(humanBytes(stats.outputBytes))
                       .arg(stats.meanBits, 0, 'f', 3));
            append(QStringLiteral("解压恢复并校验通过 · 原始文件保持不变"));
        } catch (const std::exception &error) {
            status_->setText(QString::fromUtf8(error.what()));
        }
    }
    QLabel *sourceSize_, *compressedSize_, *ratio_, *mean_, *inputInfo_, *status_, *integrity_, *entropy_,
        *hash_;
    QLineEdit *input_, *output_;
    QPushButton *sample_, *chooseInput_, *chooseOutput_, *compress_, *decompress_, *cancel_;
    QProgressBar *progress_;
    QPlainTextEdit *log_;
    FrequencyView *frequency_;
    QTableWidget *codes_;
    std::shared_ptr<WorkControl> control_;
    std::unique_ptr<QTemporaryDir> sampleDir_;
    QFutureWatcher<Outcome> watcher_;
    QTimer poll_;
    bool wasDecode_ = false;
};
} // namespace
QMainWindow *createHuffmanWindow(bool showcase, const QString &scenario) {
    return new HuffmanWindow(showcase, scenario);
}
} // namespace liss
