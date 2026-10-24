#include "common.h"
#include "os_core.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QSpinBox>
#include <QSplitter>
#include <QTimer>
#include <QToolTip>
#include <algorithm>

namespace liss {
namespace {
class MemoryView : public QWidget {
  public:
    explicit MemoryView(OsModel *model) : model_(model) {
        setMinimumHeight(225);
        setMouseTracking(true);
        setAccessibleName(QStringLiteral("内存分布图，每个方块代表一个模拟内存块"));
    }

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const int count = static_cast<int>(model_->memory().size());
        const QRectF strip(42, 5, width() - 48, 28);
        p.fillRect(strip, QColor(QStringLiteral("#202f43")));
        for (const auto &[id, process] : model_->processes()) {
            const qreal x = strip.x() + strip.width() * process.start / count,
                        w = strip.width() * process.blocks / count;
            p.fillRect(QRectF(x, strip.y(), w, strip.height()), processColor(id));
            if (w > 33) {
                p.setPen(QColor(QStringLiteral("#0a202b")));
                p.drawText(QRectF(x, strip.y(), w, strip.height()), Qt::AlignCenter,
                           QStringLiteral("P%1").arg(id));
            }
        }
        const int columns = count <= 256 ? 16 : 32;
        const int rows = (count + columns - 1) / columns;
        const qreal cellW = (width() - 48.0) / columns, cellH = std::min(29.0, (height() - 61.0) / rows);
        p.setFont(QFont(QStringLiteral("Consolas"), 9));
        for (int row = 0; row < rows; ++row) {
            p.setPen(QColor(QStringLiteral("#8ca1bb")));
            p.drawText(QRectF(0, 51 + row * cellH, 34, cellH), Qt::AlignVCenter | Qt::AlignRight,
                       QString::number(row * columns).rightJustified(3, QChar(u'0')));
            for (int col = 0; col < columns; ++col) {
                const int index = row * columns + col;
                if (index >= count)
                    break;
                const int id = model_->memory()[index];
                const QRectF r(42 + col * cellW, 51 + row * cellH, cellW - 4, cellH - 4);
                QColor color = id ? processColor(id) : QColor(QStringLiteral("#2b3c51"));
                p.setPen(QPen(color.darker(160), 1));
                QColor fill = color;
                fill.setAlpha(id ? 55 : 100);
                p.setBrush(fill);
                p.drawRoundedRect(r, 4, 4);
                if (id && cellW >= 28 && cellH >= 20) {
                    p.setPen(color);
                    p.drawText(r, Qt::AlignCenter, QString::number(id));
                }
            }
        }
    }
    void mouseMoveEvent(QMouseEvent *e) override {
        const int count = static_cast<int>(model_->memory().size()), columns = count <= 256 ? 16 : 32,
                  rows = (count + columns - 1) / columns;
        const qreal cw = (width() - 48.0) / columns, ch = std::min(29.0, (height() - 61.0) / rows);
        const int col = static_cast<int>((e->position().x() - 42) / cw),
                  row = static_cast<int>((e->position().y() - 51) / ch);
        if (e->position().x() < 42 || e->position().y() < 51 || col < 0 || col >= columns || row < 0 ||
            row >= rows || row * columns + col >= count)
            return;
        const int address = row * columns + col, id = model_->memory()[address];
        QToolTip::showText(
            e->globalPosition().toPoint(),
            id ? QStringLiteral("块 %1 · P%2 · %3").arg(address).arg(id).arg(model_->processes().at(id).name)
               : QStringLiteral("块 %1 · 空闲").arg(address),
            this);
    }

  private:
    OsModel *model_;
};
class QueueView : public QWidget {
  public:
    explicit QueueView(OsModel *model) : model_(model) {
        setMinimumHeight(131);
        setAccessibleName(QStringLiteral("运行、就绪和阻塞队列"));
    }

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        std::vector<std::vector<int>> lines(3);
        if (model_->running())
            lines[0].push_back(*model_->running());
        lines[1].assign(model_->ready().begin(), model_->ready().end());
        lines[2].assign(model_->blocked().begin(), model_->blocked().end());
        const QStringList names{QStringLiteral("RUN"), QStringLiteral("READY"), QStringLiteral("WAIT")};
        for (int row = 0; row < 3; ++row) {
            const qreal y = 3 + row * 43;
            p.setPen(QColor(QStringLiteral("#95aac5")));
            p.drawText(QRectF(0, y, 64, 33), Qt::AlignVCenter, names[row]);
            if (lines[row].empty()) {
                p.setPen(QColor(QStringLiteral("#72849e")));
                p.drawText(QRectF(76, y, width() - 80, 33), Qt::AlignVCenter, QStringLiteral("空"));
                continue;
            }
            const int visible =
                std::min(static_cast<int>(lines[row].size()), std::max(1, (width() - 80) / 88));
            for (int i = 0; i < visible; ++i) {
                int id = lines[row][i];
                QRectF r(74 + i * 88, y, 78, 33);
                QColor c = processColor(id);
                p.setPen(QPen(c.darker(160)));
                QColor fill = c;
                fill.setAlpha(30);
                p.setBrush(fill);
                p.drawRoundedRect(r, 6, 6);
                p.setPen(c);
                p.drawText(r, Qt::AlignCenter, QStringLiteral("P%1").arg(id));
            }
            if (visible < lines[row].size()) {
                p.setPen(QColor(QStringLiteral("#aabbd2")));
                p.drawText(QRectF(74 + visible * 88, y, 50, 33), Qt::AlignVCenter,
                           QStringLiteral("+%1").arg(lines[row].size() - visible));
            }
        }
    }

  private:
    OsModel *model_;
};
class OsWindow : public QMainWindow {
  public:
    OsWindow(bool showcase, const QString &scenario) {
        setWindowTitle(QStringLiteral("Liss OS Lab · 可视化操作系统"));
        setMinimumSize(1100, 790);
        resize(1400, 980);
        auto *root = new QWidget;
        root->setObjectName(QStringLiteral("root"));
        setCentralWidget(root);
        auto *layout = new QVBoxLayout(root);
        layout->setContentsMargins(24, 22, 24, 18);
        layout->setSpacing(15);
        auto *top = new QHBoxLayout;
        top->addWidget(heading(QStringLiteral("LISS QT LABS  /  OPERATING SYSTEM"),
                               QStringLiteral("看见调度，理解内存"),
                               QStringLiteral("时间片轮转 · 三态进程 · 连续内存分配与紧凑")),
                       1);
        auto *sample = button(QStringLiteral("载入示例"), QStringLiteral("loadExample"));
        auto *reset = button(QStringLiteral("重置系统"), QStringLiteral("resetSystem"));
        top->addWidget(sample);
        top->addWidget(reset);
        layout->addLayout(top);
        auto *metrics = new QHBoxLayout;
        metrics->setSpacing(12);
        metrics->addWidget(
            metricCard(QStringLiteral("CPU 当前进程"), &cpu_, QStringLiteral("Round-robin / 单核模拟")));
        metrics->addWidget(metricCard(QStringLiteral("模拟内存占用"), &memoryValue_,
                                      QStringLiteral("每块 4 KiB · 总容量 512 KiB")));
        metrics->addWidget(
            metricCard(QStringLiteral("空闲区域"), &free_, QStringLiteral("空闲合并由内存分布直接计算")));
        metrics->addWidget(
            metricCard(QStringLiteral("模拟时钟"), &clock_, QStringLiteral("ticks / 与真实系统隔离")));
        layout->addLayout(metrics);
        auto *split = new QSplitter;
        split->setChildrenCollapsible(false);
        auto *main = new QWidget;
        auto *mainLayout = new QVBoxLayout(main);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(14);
        QVBoxLayout *memLayout;
        auto *memPanel =
            panel(QStringLiteral("物理内存视图"),
                  QStringLiteral("颜色对应进程；数字为进程 ID；深色方块为空闲。鼠标停留可查看块地址。"),
                  &memLayout);
        memoryView_ = new MemoryView(&model_);
        memLayout->addWidget(memoryView_, 1);
        mainLayout->addWidget(memPanel, 3);
        QVBoxLayout *tableLayout;
        auto *tablePanel = panel(QStringLiteral("进程控制块 · PCB"),
                                 QStringLiteral("选择进程后，可唤醒阻塞项或结束该进程。"), &tableLayout);
        table_ = new QTableWidget;
        table_->setObjectName(QStringLiteral("processTable"));
        prepareTable(table_,
                     {QStringLiteral("进程"), QStringLiteral("任务名称"), QStringLiteral("状态"),
                      QStringLiteral("首地址"), QStringLiteral("占用块"), QStringLiteral("CPU 剩余")});
        table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        tableLayout->addWidget(table_, 1);
        mainLayout->addWidget(tablePanel, 3);
        QVBoxLayout *queueLayout;
        auto *queuePanel =
            panel(QStringLiteral("调度队列"),
                  QStringLiteral("就绪队列按 FIFO 排列；每个时间片执行后移至队尾。"), &queueLayout);
        queueView_ = new QueueView(&model_);
        queueLayout->addWidget(queueView_);
        mainLayout->addWidget(queuePanel, 2);
        split->addWidget(main);
        auto *side = new QWidget;
        side->setMinimumWidth(300);
        side->setMaximumWidth(410);
        auto *sideLayout = new QVBoxLayout(side);
        sideLayout->setContentsMargins(0, 0, 0, 0);
        sideLayout->setSpacing(14);
        QVBoxLayout *createLayout;
        auto *createPanel =
            panel(QStringLiteral("创建进程"), QStringLiteral("从最大空闲区域分配连续内存。"), &createLayout);
        auto *form = new QFormLayout;
        form->setSpacing(9);
        name_ = new QLineEdit(QStringLiteral("Worker"));
        name_->setMaxLength(40);
        name_->setAccessibleName(QStringLiteral("进程名称"));
        blocks_ = new QSpinBox;
        blocks_->setRange(1, 128);
        blocks_->setValue(12);
        blocks_->setSuffix(QStringLiteral(" 块"));
        burst_ = new QSpinBox;
        burst_->setRange(1, 1000000);
        burst_->setValue(24);
        burst_->setSuffix(QStringLiteral(" ticks"));
        form->addRow(QStringLiteral("名称"), name_);
        auto *memoryCpu = new QHBoxLayout;
        memoryCpu->setSpacing(8);
        memoryCpu->addWidget(blocks_);
        memoryCpu->addWidget(burst_);
        form->addRow(QStringLiteral("内存/CPU"), memoryCpu);
        createLayout->addLayout(form);
        autoCompact_ = new QCheckBox(QStringLiteral("需要时自动紧凑内存"));
        autoCompact_->setChecked(true);
        createLayout->addWidget(autoCompact_);
        auto *create = button(QStringLiteral("创建并加入调度"), QStringLiteral("createProcess"), true);
        createLayout->addWidget(create);
        sideLayout->addWidget(createPanel);
        QVBoxLayout *controlLayout;
        auto *controlPanel = panel(QStringLiteral("调度控制"),
                                   QStringLiteral("手动单步，或按固定节奏自动推进。"), &controlLayout);
        auto *quantumForm = new QFormLayout;
        quantum_ = new QSpinBox;
        quantum_->setRange(1, 1000);
        quantum_->setValue(3);
        quantum_->setSuffix(QStringLiteral(" ticks"));
        quantumForm->addRow(QStringLiteral("时间片"), quantum_);
        controlLayout->addLayout(quantumForm);
        auto *stepRow = new QHBoxLayout;
        step_ = button(QStringLiteral("单步调度"), QStringLiteral("stepSchedule"), true);
        run_ = button(QStringLiteral("自动运行"), QStringLiteral("autoRun"));
        stepRow->addWidget(step_);
        stepRow->addWidget(run_);
        controlLayout->addLayout(stepRow);
        block_ = button(QStringLiteral("阻塞运行进程"), QStringLiteral("blockRunning"));
        wake_ = button(QStringLiteral("唤醒选中进程"), QStringLiteral("wakeSelected"));
        kill_ = button(QStringLiteral("结束选中进程"), QStringLiteral("terminateSelected"));
        auto *compact = button(QStringLiteral("执行内存紧凑"), QStringLiteral("compactMemory"));
        auto *controlGrid = new QGridLayout;
        controlGrid->setSpacing(8);
        controlGrid->addWidget(block_, 0, 0);
        controlGrid->addWidget(wake_, 0, 1);
        controlGrid->addWidget(kill_, 1, 0);
        controlGrid->addWidget(compact, 1, 1);
        controlLayout->addLayout(controlGrid);
        sideLayout->addWidget(controlPanel);
        QVBoxLayout *logLayout;
        auto *logPanel =
            panel(QStringLiteral("事件轨迹"), QStringLiteral("按模拟时间记录状态转换"), &logLayout);
        log_ = new QListWidget;
        log_->setObjectName(QStringLiteral("eventLog"));
        log_->setMinimumHeight(100);
        log_->setWordWrap(true);
        logLayout->addWidget(log_, 1);
        sideLayout->addWidget(logPanel, 1);
        split->addWidget(side);
        split->setSizes({950, 340});
        layout->addWidget(split, 1);
        status_ = text(QStringLiteral("就绪"), QStringLiteral("muted"));
        status_->setAccessibleName(QStringLiteral("操作状态"));
        layout->addWidget(status_);
        connect(sample, &QPushButton::clicked, this, [this] {
            timer_.stop();
            model_.showcase();
            refresh(QStringLiteral("示例已载入，可继续创建、阻塞、唤醒或紧凑内存。"));
        });
        connect(reset, &QPushButton::clicked, this, [this] {
            timer_.stop();
            model_.reset();
            refresh(QStringLiteral("系统已重置。"));
        });
        connect(create, &QPushButton::clicked, this, [this] {
            try {
                model_.create(name_->text(), blocks_->value(), burst_->value(), autoCompact_->isChecked());
                refresh(QStringLiteral("进程已创建。"));
            } catch (const std::exception &e) {
                refresh(QString::fromUtf8(e.what()), true);
            }
        });
        connect(step_, &QPushButton::clicked, this, [this] {
            model_.step(quantum_->value());
            refresh();
        });
        connect(run_, &QPushButton::clicked, this, [this] {
            if (timer_.isActive())
                timer_.stop();
            else
                timer_.start(700);
            refresh();
        });
        connect(block_, &QPushButton::clicked, this, [this] {
            model_.block();
            refresh();
        });
        connect(wake_, &QPushButton::clicked, this, [this] {
            model_.wake(selected());
            refresh();
        });
        connect(kill_, &QPushButton::clicked, this, [this] {
            model_.terminate(selected());
            refresh();
        });
        connect(compact, &QPushButton::clicked, this, [this] {
            model_.compact();
            refresh(QStringLiteral("内存紧凑完成。"));
        });
        connect(table_, &QTableWidget::itemSelectionChanged, this, [this] { updateButtons(); });
        connect(&timer_, &QTimer::timeout, this, [this] {
            if (!model_.step(quantum_->value()))
                timer_.stop();
            refresh();
        });
        if (showcase) {
            model_.showcase();
            if (scenario == QStringLiteral("compacted"))
                model_.compact();
            if (scenario == QStringLiteral("finished")) {
                for (int i = 0; i < 100; ++i) {
                    if (!model_.step()) {
                        if (model_.blocked().empty())
                            break;
                        model_.wake(model_.blocked().front());
                    }
                }
            }
        }
        refresh(QStringLiteral("模拟器仅操作内存模型，不会创建或结束系统进程。"));
    }

  private:
    int selected() const {
        int row = table_->currentRow();
        return row >= 0 && table_->item(row, 0) ? table_->item(row, 0)->data(Qt::UserRole).toInt() : -1;
    }
    void updateButtons() {
        const int id = selected();
        kill_->setEnabled(model_.processes().contains(id));
        wake_->setEnabled(model_.processes().contains(id) &&
                          model_.processes().at(id).state == State::Blocked);
        block_->setEnabled(model_.running().has_value());
        step_->setEnabled(model_.running().has_value() && !timer_.isActive());
        run_->setText(timer_.isActive() ? QStringLiteral("暂停运行") : QStringLiteral("自动运行"));
        run_->setEnabled(timer_.isActive() || model_.running().has_value());
    }
    void refresh(const QString &message = {}, bool error = false) {
        int id = selected();
        table_->blockSignals(true);
        table_->setRowCount(static_cast<int>(model_.processes().size()));
        int row = 0;
        for (const auto &[pid, p] : model_.processes()) {
            setCell(table_, row, 0, QStringLiteral("P%1").arg(pid), processColor(pid));
            table_->item(row, 0)->setData(Qt::UserRole, pid);
            setCell(table_, row, 1, p.name);
            setCell(table_, row, 2, stateName(p.state),
                    p.state == State::Running   ? QColor(QStringLiteral("#67e3d2"))
                    : p.state == State::Blocked ? QColor(QStringLiteral("#f0c37b"))
                                                : QColor(QStringLiteral("#a7bffc")));
            setCell(table_, row, 3, QString::number(p.start));
            setCell(table_, row, 4, QString::number(p.blocks));
            setCell(table_, row, 5, QStringLiteral("%1 ticks").arg(p.remaining));
            if (pid == id)
                table_->selectRow(row);
            ++row;
        }
        table_->blockSignals(false);
        cpu_->setText(model_.running() ? QStringLiteral("P%1").arg(*model_.running())
                                       : QStringLiteral("IDLE"));
        memoryValue_->setText(QStringLiteral("%1 / %2")
                                  .arg(model_.memory().size() - model_.freeBlocks())
                                  .arg(model_.memory().size()));
        free_->setText(
            QStringLiteral("%1 段 / %2 块").arg(model_.freeRegions().size()).arg(model_.freeBlocks()));
        clock_->setText(QStringLiteral("%1").arg(model_.elapsed()));
        log_->clear();
        const auto &events = model_.events();
        for (auto i = events.rbegin(); i != events.rend(); ++i)
            log_->addItem(*i);
        memoryView_->update();
        queueView_->update();
        updateButtons();
        if (!message.isEmpty()) {
            status_->setText(message);
            status_->setProperty("kind", error ? QStringLiteral("error") : QStringLiteral("muted"));
            status_->style()->unpolish(status_);
            status_->style()->polish(status_);
        }
    }
    OsModel model_;
    QTimer timer_;
    MemoryView *memoryView_;
    QueueView *queueView_;
    QTableWidget *table_;
    QListWidget *log_;
    QLineEdit *name_;
    QSpinBox *blocks_, *burst_, *quantum_;
    QCheckBox *autoCompact_;
    QPushButton *step_, *run_, *block_, *wake_, *kill_;
    QLabel *cpu_, *memoryValue_, *free_, *clock_, *status_;
};
} // namespace
QMainWindow *createOsWindow(bool showcase, const QString &scenario) {
    return new OsWindow(showcase, scenario);
}
} // namespace liss
