#include "setdialog.h"
#include "ui_setdialog.h"
#include<QMessageBox>

bool allow;
setDialog::setDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setDialog)
{
    allow=0;
    ui->setupUi(this);
}

setDialog::~setDialog()
{
    delete ui;
}
void setDialog::inMessage(const QString &a_decodePrefix,bool b_check1,bool b_check2,bool b_check3,bool c_check1)
{
    ui->lineEdit->setText(a_decodePrefix);
    ui->lineEdit->show();
    ui->checkBox->setChecked(b_check1);
    ui->checkBox_2->setChecked(b_check2);
    ui->checkBox_3->setChecked(b_check3);
    ui->checkBox_c_check->setChecked(!c_check1);
    on_checkBox_clicked(b_check1);
}
void setDialog::outMessage(QString &a_decodePrefix, bool &b_check1, bool &b_check2, bool &b_check3,bool &c_check1)
{
    if(allow)
    {
        a_decodePrefix = ui->lineEdit->text();
        b_check1=ui->checkBox->isChecked();
        b_check2=ui->checkBox_2->isChecked();
        b_check3=ui->checkBox_3->isChecked();
        c_check1=!ui->checkBox_c_check->isChecked();
    }
}

void setDialog::on_checkBox_clicked(bool checked)
{
    if(checked)
    {
        ui->checkBox_2->setEnabled(true);
        ui->checkBox_3->setEnabled(true);
    }
    else
    {
        ui->checkBox_2->setEnabled(false);
        ui->checkBox_3->setEnabled(false);
    }
}

void setDialog::on_buttonBox_accepted()
{
    allow=1;
}

void setDialog::on_radioButton_cradio_clicked(bool checked)
{
    if(checked)
        ui->checkBox_c_check->setChecked(true);
}

void setDialog::on_radioButton_cradio2_clicked(bool checked)
{
    if(checked)
        ui->checkBox_c_check->setChecked(false);
}

void setDialog::on_checkBox_c_check_clicked()
{
    ui->radioButton_cradio2_2->setChecked(1);
}
