#include "input_createprocessdialog.h"
#include "ui_input_createprocessdialog.h"

input_CreateProcessDialog::input_CreateProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::input_CreateProcessDialog)
{
    ui->setupUi(this);
    QRegExp regExp("^(([0-9]+\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\.[0-9]+)|([0-9]*[1-9][0-9]*))$");//正浮点数
    ui->lineEdit_2->setValidator(new QRegExpValidator(regExp, this));
    ui->lineEdit->setFocus();
}

input_CreateProcessDialog::~input_CreateProcessDialog()
{
    delete ui;
}

void input_CreateProcessDialog::on_buttonBox_accepted()
{
    name=ui->lineEdit->text();
    size=ui->lineEdit_2->text().toFloat();
}
