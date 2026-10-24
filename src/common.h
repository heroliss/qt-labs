#pragma once
#include <QColor>
#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace liss {
void applyTheme();
QColor processColor(int id);
QFrame *panel(const QString &title, const QString &subtitle, QVBoxLayout **content = nullptr);
QFrame *metricCard(const QString &caption, QLabel **value, const QString &detail = {});
QFrame *heading(const QString &eyebrow, const QString &title, const QString &subtitle);
QPushButton *button(const QString &text, const QString &name, bool primary = false);
QLabel *text(const QString &value, const QString &kind = {}, bool wrap = true);
void prepareTable(QTableWidget *table, const QStringList &columns);
void setCell(QTableWidget *table, int row, int column, const QString &value, const QColor &color = {});
QMainWindow *createOsWindow(bool showcase, const QString &scenario);
QMainWindow *createHuffmanWindow(bool showcase, const QString &scenario);
QMainWindow *createCalcWindow(bool showcase, const QString &scenario);
} // namespace liss
