#ifndef INPUT_CREATEPROCESSDIALOG_H
#define INPUT_CREATEPROCESSDIALOG_H

#include <QDialog>

namespace Ui {
class input_CreateProcessDialog;
}

class input_CreateProcessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit input_CreateProcessDialog(QWidget *parent = 0);
    ~input_CreateProcessDialog();
    QString name;
    float size;
private slots:
    void on_buttonBox_accepted();

private:
    Ui::input_CreateProcessDialog *ui;

};

#endif // INPUT_CREATEPROCESSDIALOG_H
