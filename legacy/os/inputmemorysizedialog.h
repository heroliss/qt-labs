#ifndef INPUTMEMORYSIZEDIALOG_H
#define INPUTMEMORYSIZEDIALOG_H

#include <QDialog>

namespace Ui {
class inputMemorySizeDialog;
}

class inputMemorySizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit inputMemorySizeDialog(QWidget *parent = 0);
    ~inputMemorySizeDialog();
    int blockSize;
    int blockNum;
    double product;
private slots:
    void on_buttonBox_accepted();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_2_textChanged(const QString &arg1);

private:


    Ui::inputMemorySizeDialog *ui;
};

#endif // INPUTMEMORYSIZEDIALOG_H
