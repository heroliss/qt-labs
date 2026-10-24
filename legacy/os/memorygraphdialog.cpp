#include "memorygraphdialog.h"
#include "ui_memorygraphdialog.h"

MemoryGraphDialog::MemoryGraphDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MemoryGraphDialog)
{
    ui->setupUi(this);
    painter=new QPainter();

}

MemoryGraphDialog::~MemoryGraphDialog()
{
    delete ui;
}

void MemoryGraphDialog::paintEvent(QPaintEvent *)
{
    painter->begin(this);
    int blockNum=os->getMemory()->getBlockNum();
    Qt::GlobalColor color;
    char colorIndex;
    int i;

    int y=0;
    int t=0;
    for(i=0;i<blockNum;i++)
    {
        colorIndex =os->getMemory()->at(i);
        //以下是颜色值的转换
        if(colorIndex==0)
            color=Qt::white;
        else
            switch((unsigned)colorIndex%13+1)
            {
            case 1:color=Qt::red;break;
            case 2:color=Qt::darkYellow;break;
            case 3:color=Qt::blue;break;
            case 4:color=Qt::darkCyan;break;
            case 5:color=Qt::darkRed;break;
            case 6:color=Qt::yellow;break;
            case 7:color=Qt::green;break;
            case 8:color=Qt::magenta;break;
            case 9:color=Qt::darkBlue;break;
            case 10:color=Qt::darkMagenta;break;
            case 11:color=Qt::cyan;break;
            case 12:color=Qt::darkGreen;break;
            case 13:color=Qt::gray;break;
            }
        painter->setBrush(QBrush(color,Qt::SolidPattern));
        if(i%(this->geometry().width()/5)==0)
        {
            t=i;
            y++;
        }
        painter->drawRect((i-t)*5,1+y*20,5,20);
    }
    painter->end();
}
