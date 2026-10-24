#include "inputmemorysizedialog.h"
#include "ui_inputmemorysizedialog.h"

inputMemorySizeDialog::inputMemorySizeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::inputMemorySizeDialog)
{
    ui->setupUi(this);
    QRegExp regExp("^[0-9]{5}");
    QRegExp regExp2("^[0-9]{6}");
    ui->lineEdit->setValidator(new QRegExpValidator(regExp, this));
    ui->lineEdit_2->setValidator(new QRegExpValidator(regExp2, this));
    ui->lineEdit->setText("1024");
    ui->lineEdit_2->setText("1024");
    blockNum=ui->lineEdit_2->text().toInt();
    blockSize=ui->lineEdit->text().toInt();
    product=blockNum*blockSize;
}

inputMemorySizeDialog::~inputMemorySizeDialog()
{
    delete ui;
}

void inputMemorySizeDialog::on_lineEdit_textChanged(const QString &arg1)
{
    if(!ui->lineEdit_2->text().isEmpty())
    {
        product=ui->lineEdit->text().toInt()*ui->lineEdit_2->text().toInt();
        if(product==0)
        {
            ui->buttonBox->setEnabled(false);
            ui->label_6->setText("<font color=red>内存大小不能为0！</font>");
        }else
        {
            ui->buttonBox->setEnabled(true);
            ui->label_6->setText( QString::number(product/1024,'f',3)+"KB\n"+QString::number(product/(1024*1024),'f',3)+"MB");
        }

    }
    else
        ui->buttonBox->setEnabled(false);
}

void inputMemorySizeDialog::on_buttonBox_accepted()
{
    blockNum=ui->lineEdit_2->text().toInt();
    blockSize=ui->lineEdit->text().toInt();
}

void inputMemorySizeDialog::on_lineEdit_2_textChanged(const QString &arg1)
{
    if(!ui->lineEdit->text().isEmpty())
    {
        product=ui->lineEdit->text().toInt()*ui->lineEdit_2->text().toInt();
        if(product==0)
        {
            ui->buttonBox->setEnabled(false);
            ui->label_6->setText("<font color=red>内存大小不能为0！</font>");
        }else
        {
            ui->buttonBox->setEnabled(true);
            ui->label_6->setText( QString::number(product/1024,'f',3)+"KB\n"+QString::number(product/(1024*1024),'f',3)+"MB");
        }

    }
    else
        ui->buttonBox->setEnabled(false);
}
