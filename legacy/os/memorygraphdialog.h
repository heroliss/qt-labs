#ifndef MEMORYGRAPHDIALOG_H
#define MEMORYGRAPHDIALOG_H

#include <QDialog>
#include <QPainter>
#include "os_basic.h"
namespace Ui {
class MemoryGraphDialog;
}

class MemoryGraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MemoryGraphDialog(QWidget *parent = 0);
    ~MemoryGraphDialog();
    void setOS(OS_Basic *os){this->os=os;}
    void initMemoryGraph(){update();}
private:
    Ui::MemoryGraphDialog *ui;
    void paintEvent(QPaintEvent *);
    QPainter *painter;
    OS_Basic *os;

};

#endif // MEMORYGRAPHDIALOG_H
