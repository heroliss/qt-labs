#include "common.h"
#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QHeaderView>
#include <QPainter>
#include <QStyleFactory>

namespace liss {
void applyTheme() {
    // Load installed Windows fonts explicitly as well as using system fallback.
    // This also keeps QWidget capture rendering identical when using Qt's offscreen QPA.
    const QString fontRoot =
        qEnvironmentVariable("WINDIR", QStringLiteral("C:/Windows")) + QStringLiteral("/Fonts/");
    for (const QString &file : {QStringLiteral("msyh.ttc"), QStringLiteral("msyhbd.ttc"),
                                QStringLiteral("segoeui.ttf"), QStringLiteral("consola.ttf")})
        QFontDatabase::addApplicationFont(fontRoot + file);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPointSize(10);
    QApplication::setFont(font);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#0b1019")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#e7edf8")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#0e1521")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#e7edf8")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#192437")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#e7edf8")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#1a5562")));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#8997b0")));
    QApplication::setPalette(palette);
    qApp->setStyleSheet(QStringLiteral(R"(
      QWidget { color: #e7edf8; font-size: 13px; }
      QMainWindow, QWidget#root { background: #0b1019; }
      QFrame#panel, QFrame#metric { background: #111b29; border: 1px solid #26344a; border-radius: 12px; }
      QFrame#heading { background: transparent; border: none; }
      QLabel { background: transparent; border: none; }
      QLabel[kind="eyebrow"] { color: #62d6dd; font-size: 11px; font-weight: 600; }
      QLabel[kind="title"] { font-size: 29px; font-weight: 650; letter-spacing: -0.5px; }
      QLabel[kind="section"] { font-size: 15px; font-weight: 600; }
      QLabel[kind="muted"] { color: #a1afc6; font-size: 12px; }
      QLabel[kind="value"] { color: #f3f8ff; font-size: 25px; font-weight: 600; }
      QLabel[kind="result"] { color: #68e0d1; font-size: 40px; font-weight: 500; }
      QLabel[kind="error"] { color: #ffb6ad; }
      QPushButton { background: #1a293d; border: 1px solid #34465f; border-radius: 7px; padding: 9px 13px; font-weight: 500; min-height: 18px; }
      QPushButton:hover { background: #22364c; border-color: #628aa2; }
      QPushButton:pressed { background: #123d49; }
      QPushButton:focus { border: 2px solid #72dbe1; padding: 8px 12px; }
      QPushButton[primary="true"] { background: #67ded4; border-color: #67ded4; color: #08242c; font-weight: 650; }
      QPushButton[primary="true"]:hover { background: #8ae8e0; }
      QPushButton:disabled { background: #17202d; color: #7e8ba2; border-color: #27354a; }
      QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QListWidget { background: #0e1623; border: 1px solid #34445f; border-radius: 6px; padding: 7px; selection-background-color: #245467; }
      QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { min-height: 20px; }
      QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { border: 1px solid #68d7dc; }
      QComboBox::drop-down { border: none; width: 24px; }
      QCheckBox { spacing: 8px; color: #b7c5d8; }
      QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; border: 1px solid #65809c; background: #121e2d; }
      QCheckBox::indicator:checked { background: #54bfc3; border: 3px solid #adf7ee; }
      QTableWidget { background: #111b29; alternate-background-color: #142030; border: none; gridline-color: #223048; selection-background-color: #214556; border-radius: 4px; }
      QTableWidget::item { padding: 8px 6px; border-bottom: 1px solid #223048; }
      QHeaderView::section { background: #152236; color: #9fb2ce; border: none; padding: 9px 7px; font-size: 11px; font-weight: 600; }
      QScrollBar:vertical { background: #0d1520; width: 10px; margin: 0; }
      QScrollBar::handle:vertical { background: #39516c; border-radius: 4px; min-height: 24px; }
      QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
      QScrollBar:horizontal { background: #0d1520; height: 10px; }
      QScrollBar::handle:horizontal { background: #39516c; border-radius: 4px; min-width: 24px; }
      QTabWidget::pane { border: none; }
      QTabBar::tab { background: #152134; color: #b2c0d5; padding: 9px 18px; margin-right: 5px; border: 1px solid #2d3e57; border-radius: 6px; }
      QTabBar::tab:selected { background: #214b59; color: #8cf1e9; border-color: #47818a; }
      QProgressBar { border: 1px solid #2e4059; border-radius: 5px; background: #0d1622; text-align: center; min-height: 19px; }
      QProgressBar::chunk { background: #2e8f9c; border-radius: 4px; }
      QToolTip { color: #e7edf8; background: #20344c; border: 1px solid #688aa9; padding: 6px; }
      QSplitter::handle { background: #0b1019; }
    )"));
    QPixmap icon(64, 64);
    icon.fill(Qt::transparent);
    QPainter p(&icon);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#101d31")));
    p.drawRoundedRect(0, 0, 64, 64, 13, 13);
    p.setBrush(QColor(QStringLiteral("#6ae3db")));
    p.drawRoundedRect(14, 13, 10, 38, 3, 3);
    p.drawRoundedRect(22, 41, 28, 10, 3, 3);
    p.end();
    qApp->setWindowIcon(QIcon(icon));
}
QColor processColor(int id) {
    static const QStringList colors{QStringLiteral("#64dacf"), QStringLiteral("#7ea9ff"),
                                    QStringLiteral("#d4a0ef"), QStringLiteral("#f5be76"),
                                    QStringLiteral("#89ce87"), QStringLiteral("#ee91a0"),
                                    QStringLiteral("#80c7ed"), QStringLiteral("#c4b1ff")};
    return QColor(colors[(id - 1 + colors.size()) % colors.size()]);
}
QLabel *text(const QString &value, const QString &kind, bool wrap) {
    auto *label = new QLabel(value);
    label->setProperty("kind", kind);
    label->setWordWrap(wrap);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}
QFrame *panel(const QString &title, const QString &subtitle, QVBoxLayout **content) {
    auto *frame = new QFrame;
    frame->setObjectName(QStringLiteral("panel"));
    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(11);
    if (!title.isEmpty())
        layout->addWidget(text(title, QStringLiteral("section")));
    if (!subtitle.isEmpty())
        layout->addWidget(text(subtitle, QStringLiteral("muted")));
    if (content)
        *content = layout;
    return frame;
}
QFrame *metricCard(const QString &caption, QLabel **value, const QString &detail) {
    auto *frame = new QFrame;
    frame->setObjectName(QStringLiteral("metric"));
    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(4);
    layout->addWidget(text(caption, QStringLiteral("muted")));
    *value = text(QStringLiteral("—"), QStringLiteral("value"), false);
    layout->addWidget(*value);
    if (!detail.isEmpty())
        layout->addWidget(text(detail, QStringLiteral("muted")));
    return frame;
}
QFrame *heading(const QString &eyebrow, const QString &title, const QString &subtitle) {
    auto *frame = new QFrame;
    frame->setObjectName(QStringLiteral("heading"));
    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->addWidget(text(eyebrow, QStringLiteral("eyebrow")));
    layout->addWidget(text(title, QStringLiteral("title")));
    layout->addWidget(text(subtitle, QStringLiteral("muted")));
    return frame;
}
QPushButton *button(const QString &value, const QString &name, bool primary) {
    auto *b = new QPushButton(value);
    b->setObjectName(name);
    b->setAccessibleName(value);
    b->setCursor(Qt::PointingHandCursor);
    b->setProperty("primary", primary);
    return b;
}
void prepareTable(QTableWidget *table, const QStringList &columns) {
    table->setColumnCount(columns.size());
    table->setHorizontalHeaderLabels(columns);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(39);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setShowGrid(false);
    table->setAlternatingRowColors(true);
    table->setFocusPolicy(Qt::StrongFocus);
}
void setCell(QTableWidget *table, int row, int col, const QString &value, const QColor &color) {
    auto *item = new QTableWidgetItem(value);
    if (color.isValid())
        item->setForeground(color);
    table->setItem(row, col, item);
}
} // namespace liss
