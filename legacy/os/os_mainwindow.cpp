#include "os_mainwindow.h"
#include "ui_os_mainwindow.h"
#include "inputmemorysizedialog.h"
#include "input_createprocessdialog.h"
#include "math.h"
#include "QMessageBox"
#include "QInputDialog"
#include "time.h"
#include "QThread"
#include "QFile"

#include <QFileDialog>
#include <QProcess>

OS_MainWindow::OS_MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OS_MainWindow)
{
    inputMemorySizeDialog inputDialog;

    if(QDialog::Rejected==inputDialog.exec())
        close();
    blockSize=inputDialog.blockSize;
    blockNum=inputDialog.blockNum;
    double product=inputDialog.product;
    ui->setupUi(this);
 // QPoint p= mapToGlobal(this->mapToParent(QPoint(0,0)));//?????????????
 //   int x=p.x();
 //   int y=p.y();
    memoryDialog=new MemoryGraphDialog();
    //memoryDialog->setGeometry(x+500,y,400,300);  //???????????????

    processDialog=new ProcessTableDialog();
   // processDialog->setGeometry(x+500,y+300,400,300);

    os=new OS_Basic(blockNum,blockSize);
    processDialog->setOS(os);//向另一个窗口载入os指针
    memoryDialog->setOS(os);

    ui->label_5->setText("块大小 "+QString::number(blockSize)+"byte");
    ui->label_6->setText("块总数 "+QString::number(blockNum));
    ui->label_7->setText("容量 "+QString::number(product/(1024*1024),'f',2)+"MB");
    ui->label_8->setText("    "+QString::number(product/1024,'f',2)+"KB");
    resetMemoryLable();
    colorIndex=1; //写入内存的内容，从1开始写入，0代表空
    statusLabel=new QLabel("欢迎使用操作系统进程调度演示系统！");
    ui->statusBar->addWidget(statusLabel);
    qsrand(time(NULL));//设置随机数种子
    ui->toolButton->setVisible(false);
    ui->toolButton_2->setVisible(false);
    sleepTime=500;

    addFileToRandomNameList("随机进程名单.txt");
    sizeMax=2*blockNum/nameList.size();//这里的size都是指页面数量
    sizeMin=blockNum/nameList.size()/5;//首次启动程序启动计算一个推荐的合适的进程大小范围
    if(sizeMax==0)sizeMax++;
}

OS_MainWindow::~OS_MainWindow()
{
    delete ui;
}

void OS_MainWindow::closeEvent(QCloseEvent *)
{
    timeStoped=true;
    t.invalidate();
    memoryDialog->accept();
    processDialog->accept();
    delete (memoryDialog);
    delete (processDialog);
}

void OS_MainWindow::paintEvent(QPaintEvent *)
{
}

void OS_MainWindow::reloadStateList()//重新加载显示三个状态表
{
    vector<PCB*> *list;
    ui->listWidget->clear();
    ui->listWidget_2->clear();
    ui->listWidget_3->clear();
    list=os->getReadyList();
    QString s;
    while(!list->empty())
    {
        ui->listWidget->addItem(list->front()->name);
        list->erase(list->begin());
    }
    delete list;

    list=os->getRunIndexList();
    while(!list->empty())
    {
        ui->listWidget_2->addItem(list->front()->name);
        list->erase(list->begin());
    }
    delete list;

    vector<PCB*> *blockList;
    blockList=os->getBlockList();
    for(int i=0;i<blockList->size();i++)
        ui->listWidget_3->addItem(blockList->at(i)->name);
    delete blockList;
}

void OS_MainWindow::resetMemoryLable()
{
    QString s="可用/已用(KB) <br><font color=green>"+QString::number((float)os->getMemory()->getFreeSpaceSize()/1024,'f',2)
            +"</font>/<font color=red>"+QString::number((float)os->getMemory()->getOccupiedSpaceSize()/1024,'f',2)+"</font>";
    ui->label_9->setText(s);
}

void OS_MainWindow::AutomaticDemonstration()
{
    static unsigned pointNum=0;
    QString sPoint;
    int size;
    int operation=0; //随机操作
    vector<PCB*> *run_block_List;

    while(!timeStoped)
    {
        sPoint="自动演示";
        for(unsigned i=0;i<pointNum%4;i++)
            sPoint+=".";
        ui->pushButton_6->setText(sPoint);
        pointNum++;

        operation=qrand()%5;
        switch(operation)
        {
        case 0:
            if(!os->memoryFull()/*&&!nameList.empty()*/) //创建进程操作
            {
                on_pushButton_8_clicked();
            } break;
        case 1:
            try{ os->destroyRunningProcess();}
            catch(memory_error e){statusLabel->setText("<font color=red>尝试关闭进程失败！原因：没有正在运行的程序</font>");break;}
            statusLabel->setText("<font color=green>释放了一个正在运行的进程</font>");
            break;
        case 2:
            if(os->runToBlock())     //阻塞进程操作
                 statusLabel->setText("<font color=brown>一个进程被阻塞</font>");
            else
                 statusLabel->setText("<font color=red>尝试阻塞进程失败！原因：没有正在运行的进程</font>");
            break;
        case 3:
            run_block_List=os->getBlockList();
            if(!run_block_List->empty())
               { os->blockToReady(rand()%(run_block_List->size())); //唤醒进程操作
                 statusLabel->setText("<font color=green>唤醒了一个进程</font>");}
            else
                 statusLabel->setText("尝试唤醒一个进程，但发现没有阻塞态进程");
            delete run_block_List;break;
        case 4:
            os->timeRoundRobin(); //轮转调度操作
             statusLabel->setText("调度就绪态进程");
            break;
        }
        flashAllWindows();
        t.start();
        while(t.elapsed()<sleepTime)
        {
           QCoreApplication::processEvents();
        }
    }
}

void OS_MainWindow::addFileToRandomNameList(QString path)
{
    nameList.clear();
    QFile file;
    file.setFileName(path);
    if(!file.open(QFile::ReadOnly))
    {
        statusLabel->setText("<font color=red>打开随机名单文件失败！</font>");
        nameList.push_back("<请重新打开一个随机名单文件>");
        return;
    }
    char c[100];
    QString s;
    while(!file.atEnd())
    {
        file.readLine(c,100);
        s=s.fromLocal8Bit(c);
        s.remove(s.size()-1,1);
        nameList.push_back(s);
    }

}

void OS_MainWindow::flashAllWindows()
{
    resetMemoryLable();//重设内存标签信息
    reloadStateList();//重新加载三个状态表
    memoryDialog->update();//刷新内存图式窗口
    processDialog->initTable();//刷新进程列表窗口
}

void OS_MainWindow::on_action_triggered()
{
    memoryDialog->show();
    memoryDialog->update();
}

void OS_MainWindow::on_action_4_triggered()
{
    this->close();
}

void OS_MainWindow::on_action_5_triggered() //打开pcb信息表
{
    processDialog->initTable();//初始化pcb信息表
    processDialog->show();
}

void OS_MainWindow::on_pushButton_clicked()//创建进程
{
    input_CreateProcessDialog dialog;
    if(QDialog::Rejected==dialog.exec())
        return;
    if(dialog.name==""){
        statusLabel->setText("<font color=brown>进程创建已取消</font> (没有输入进程名！)");return; }
    if(dialog.size==0){
        statusLabel->setText("<font color=brown>进程创建已取消</font> (进程大小不能为0！)");return; }

    try{if(colorIndex==0) colorIndex++;
        os->createProcess(dialog.name, (int)ceil((dialog.size*1024)/blockSize),colorIndex++);}//创建进程的同时颜色索引加一
    catch(memory_error e){statusLabel->setText("<font color=red>创建进程错误</font> (内存不足！)");return;}
    statusLabel->setText("进程创建成功！");
    flashAllWindows();
}

void OS_MainWindow::on_pushButton_2_clicked()//关闭进程
{
    try{os->destroyRunningProcess();}
    catch(memory_error e)
    {statusLabel->setText("<font color=brown>没有正在运行的进程！</font>");return;}
    statusLabel->setText("关闭进程");
    flashAllWindows();
}
OS_Basic *OS_MainWindow::getOs() const
{
    return os;
}

void OS_MainWindow::setOs(OS_Basic *value)
{
    os = value;
}


void OS_MainWindow::on_pushButton_3_clicked() //阻塞进程
{
    if(!os->runToBlock())
    {statusLabel->setText("<font color=red>阻塞进程失败</font> (没有正在运行的进程！)");return;}
    reloadStateList();
    resetMemoryLable();//此句当前无用
    statusLabel->setText("阻塞进程");
    processDialog->initTable();
}

void OS_MainWindow::on_pushButton_4_clicked() //唤醒进程
{
    bool isOK;
    int text = QInputDialog::getInt( NULL, "唤醒进程",
                                     "请在这里输入您要唤醒的阻塞态进程的序号:\n(提示：您也可以直接双击阻塞态列表里的进程)"
                                     ,0,0,99999,1,&isOK);
    if(isOK&&!os->blockToReady(text))
    {statusLabel->setText("<font color=red>唤醒进程失败</font> (没有找到该进程！您输入的序号有误！)");return;}
    reloadStateList();
    resetMemoryLable();//此句当前无用
    statusLabel->setText("唤醒进程");
    processDialog->initTable();
}

void OS_MainWindow::on_pushButton_5_clicked() //时间片轮转
{
    os->timeRoundRobin();
    reloadStateList();
    resetMemoryLable();//此句当前无用
    statusLabel->setText("完成一次进程调度");
    processDialog->initTable();
}

void OS_MainWindow::on_pushButton_6_clicked() //自动演示
{
    bool ok;
    int num = QInputDialog::getInt(this,"自动演示时间步长设置","请设置时间间隔(ms)\n(输入0表示手动单步执行):",sleepTime,0,9999999,100,&ok);
    if(num==0)
        num=9999999;
    if(ok){
        sleepTime=num;
    }else return;

    timeStoped=false;
    ui->toolButton->setVisible(true);
    ui->toolButton_2->setVisible(true);
    ui->toolButton_2->setText("暂停");
    AutomaticDemonstration();
}

void OS_MainWindow::on_toolButton_2_clicked() //暂停
{
    if(true==timeStoped){
        ui->toolButton->setVisible(false);
        ui->toolButton_2->setVisible(false);
        ui->toolButton_2->setText("暂停");
    }
    else{
        ui->toolButton_2->setText("停止");
    }
    timeStoped=true;
    t.invalidate();  //设置间隔计时器为无效的来跳出处理所有程序时间的循环，否则会陷入递归调用
}

void OS_MainWindow::on_toolButton_clicked()  //继续
{
    ui->toolButton_2->setText("暂停");
    timeStoped=false;
    t.invalidate();
    AutomaticDemonstration();
}

void OS_MainWindow::on_listWidget_3_doubleClicked(const QModelIndex &index)
{
    if(!os->blockToReady(ui->listWidget_3->currentRow()))
    {statusLabel->setText("<font color=red>唤醒进程失败</font>");return;}
    statusLabel->setText("唤醒进程");
    flashAllWindows();
}

void OS_MainWindow::on_listWidget_doubleClicked(const QModelIndex &index)
{
    on_pushButton_5_clicked();
}

void OS_MainWindow::on_listWidget_2_doubleClicked(const QModelIndex &index)
{
    on_pushButton_3_clicked();
}

void OS_MainWindow::on_pushButton_8_clicked() //自动创建
{
    int size=qrand()%(sizeMax-sizeMin)+sizeMin; //获取一个合适的随机大小
    if(size==0) size++;
    int nameIndex=qrand()%nameList.size(); //获取一个随机名字序号
    try{if(colorIndex==0) colorIndex++;
    os->createProcess(nameList.at(nameIndex),size,colorIndex++);}
    catch (memory_error e){statusLabel->setText("<font color=red>尝试创建进程失败！原因：内存不足</font>"); return;}
    statusLabel->setText("<font color=green>成功创建了一个进程</font>");
    flashAllWindows();
}

void OS_MainWindow::on_pushButton_7_clicked() //内存整理
{
    os->compactMemory();
    flashAllWindows();
}

void OS_MainWindow::on_action_7_triggered() //设置
{
    bool ok;
    int num = QInputDialog::getInt(this,"自动创建进程大小范围设置","请设置自动生成的进程大小下限:\t\t",sizeMin,0,9999999,100,&ok);
    if(ok){
        sizeMin=num;
    }else return;
    if(sizeMax==0) sizeMax++;
    num = QInputDialog::getInt(this,"自动创建进程大小范围设置","请设置上限:\t\t\t\t",sizeMax,sizeMin+1,9999999,100,&ok);
    if(ok){
        sizeMax=num;
    }else return;
}

void OS_MainWindow::on_action_8_triggered() //关于
{
    QMessageBox::information(this,"关于","模拟操作系统进程管理演示系统\n\t2014.10.5\n\tbuilt by ss");
}

void OS_MainWindow::on_action_3_triggered() //帮助
{
    QMessageBox::information(this,"帮助提示",
                             QString("1.本软件模拟了内存的写入和读取等操作\n")
                             +"2.基于上一点，不建议您设置太大的内存块数\n"
                             +"3.自动演示过程中，可以执行其他任何操作\n"
                             +"4.自动演示过程中，点击“继续”按钮可以加速执行\n");
}

void OS_MainWindow::on_action_6_triggered() //打开控制台
{
    if(!QProcess::startDetached("os交互系统.exe",QStringList()))
        QMessageBox::warning(this,"错误","打开控制台失败！");
}

void OS_MainWindow::on_action_2_triggered() //载入随机名单
{
     QString fileName = QFileDialog::getOpenFileName(this, "打开随机进程名单文件","","文本文件(*.txt);;Allfile(*.*)");
     if(fileName=="") return;
     statusLabel->setText(QString("成功载入随机进程名单文件")+fileName);
     addFileToRandomNameList(fileName);
}

void OS_MainWindow::on_action_QT_triggered()
{
    QMessageBox::aboutQt(this,"编译器信息");
}
