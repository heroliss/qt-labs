#ifndef PROCESSTABLEDIALOG_H
#define PROCESSTABLEDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "os_basic.h"
namespace Ui {
class ProcessTableDialog;
}

class ProcessTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProcessTableDialog(QWidget *parent = 0);
    ~ProcessTableDialog();
    void setOS(OS_Basic *os){this->os=os;}
    void initTable();
private:
    Ui::ProcessTableDialog *ui;
    OS_Basic *os;
    QStandardItemModel  *model;

};

#endif // PROCESSTABLEDIALOG_H
