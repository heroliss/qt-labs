#include "processtabledialog.h"
#include "ui_processtabledialog.h"

ProcessTableDialog::ProcessTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessTableDialog)
{
    ui->setupUi(this);
    model = new QStandardItemModel();//添加表头
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);//设置非编辑状态
}

ProcessTableDialog::~ProcessTableDialog()
{
    delete ui;
}

void ProcessTableDialog::initTable()
{
    model->clear();

    model->setColumnCount(6);
    model->setHeaderData(0,Qt::Horizontal,QString("外部标识"));
    model->setHeaderData(1,Qt::Horizontal,QString("状态"));
    model->setHeaderData(2,Qt::Horizontal,QString("优先级"));
    model->setHeaderData(3,Qt::Horizontal,QString("段首地址(页)"));
    model->setHeaderData(4,Qt::Horizontal,QString("页总数"));
    model->setHeaderData(5,Qt::Horizontal,QString("大小"));
    //ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);//表头信息显示居左
    ui->tableView->setModel(model);
    map<int ,PCB*> *PCBList=os->getPCBlist();
    map<int ,PCB*>::iterator it;
    int i=0;
    QString s;
    PCB* pcb;
    for(it = PCBList->begin(); it !=PCBList->end(); it++)
    {
        pcb=it->second;
        model->setItem(i,0,new QStandardItem(pcb->name));
       // model->item(i,0)->setForeground(QBrush(QColor(0, 127, 0))); //设置字符颜色
        model->item(i,0)->setTextAlignment(Qt::AlignCenter); //设置字符位置
        switch(pcb->state)
        {
        case Ready:
            s="就绪";
            model->setItem(i,1,new QStandardItem(s));
            model->item(i,1)->setForeground(QBrush(QColor(0, 200, 0)));break;
        case Blocked:
            s="阻塞";
            model->setItem(i,1,new QStandardItem(s));
            model->item(i,1)->setForeground(QBrush(QColor(255, 0, 0)));break;
        case Run:
            s="执行中";
            model->setItem(i,1,new QStandardItem(s));
            model->item(i,1)->setForeground(QBrush(QColor(0, 0, 200)));break;  //状态
        default:break;
        }
        model->item(i,1)->setTextAlignment(Qt::AlignCenter);
        switch(pcb->priority)
        {
        case Highest:
            s="最高";break;
        case High:
            s="较高";break;
        case Middle:
            s="正常";break;
        case Low:
            s="较低";break;
        case Lowest:
            s="最低";break;
        default:break;
        }
        model->setItem(i,2,new QStandardItem(s));//优先级
        model->item(i,2)->setTextAlignment(Qt::AlignCenter);//字体位置
        model->setItem(i,3,new QStandardItem(QString::number(pcb->segment_address)));//段首地址
        model->setItem(i,4,new QStandardItem(QString::number(pcb->pages_amount)));//页总数
        model->setItem(i,5,new QStandardItem(QString::number( (float)(pcb->pages_amount) * (os->getMemory()->getBlockSize())/1024 ,'f',2)+"KB"));
        i++;
    }
}
