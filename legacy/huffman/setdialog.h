#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QDialog>

namespace Ui {
class setDialog;
}

class setDialog : public QDialog
{
    Q_OBJECT

public:
    explicit setDialog(QWidget *parent = 0);
    ~setDialog();
    void inMessage(const QString &a_decodePrefix, bool b_check1, bool b_check2, bool b_check3, bool c_check1);
    void outMessage(QString &a_decodePrefix, bool &b_check1, bool &b_check2, bool &b_check3,bool &c_check1);

private slots:
    void on_checkBox_clicked(bool checked);

    void on_buttonBox_accepted();

    void on_radioButton_cradio_clicked(bool checked);

    void on_radioButton_cradio2_clicked(bool checked);

    void on_checkBox_c_check_clicked();

private:
    Ui::setDialog *ui;
};

#endif // SETDIALOG_H
