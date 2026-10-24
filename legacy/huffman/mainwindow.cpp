#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"setdialog.h"
#include"QFile"
#include<QString>
#include"QMessageBox"
#include"qfiledialog.h"
#include"huffman.h"
#include<QTextCodec>
#include<time.h>
#include<QMimeData>
#include<QFileInfo>
#include<QStack>
using namespace std;

int tag1=0,tag2=0;
bool c_check1=1;
QString decodePrifix="[解码]";
string route1,route2;//译码本和编码文件路径
QString information,sfile;
QStack<QString> saveInformation;
fstream sf;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->plainTextEdit->setAcceptDrops(true);
    setAcceptDrops(true);
    sfile="";

}


MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasText())
    {
        event->acceptProposedAction();
    }
}
void MainWindow::dropEvent(QDropEvent *event)
{

    if(event->mimeData()->hasText())
    {
        QString s=event->mimeData()->text();
        s.remove(0,8);
        QFileInfo fileinfo(s);
        if(fileinfo.isDir())
            return;
        event->acceptProposedAction();
        QString st=getFileType(s);
        if(st=="hcode")
        {
            addInformation("拖入编码文件："+getFilename(s));
            route2=s.toStdString();
            on_pushButton_4_clicked(1);
        }
        else if(st=="hkey")
        {
            addInformation("拖入译码本："+getFilename(s));
            route1=s.toStdString();
            on_pushButton_3_clicked(1);
        }
        else
        {
            addInformation("拖入原文件："+getFilename(s));
            sfile=s;
            on_pushButton_open_clicked(1);
        }

    }

}
void MainWindow::addInformation(const QString &s)
{
    information+=s+"\n";
    ui->plainTextEdit->setPlainText(information);
}

QString MainWindow::turnFileSize_toQString(float filesize_num)
{
    QString fileSize=QString::number(filesize_num);
    fileSize+="byte";
    if(filesize_num>1024)
    {
        filesize_num=filesize_num/1024;
        fileSize=QString::number(filesize_num,'f',2);
        fileSize+="KB";
    }
    if(filesize_num>1024)
    {
        filesize_num=filesize_num/1024;
        fileSize=QString::number(filesize_num,'f',2);
        fileSize+="MB";
    }
    return fileSize;
}
void MainWindow::on_pushButton_clicked()
{
    if(!sf.is_open())
    {
        QMessageBox::information(this,"提示","请先打开原文件",QMessageBox::Ok);
        sfile="";
        return;
    }
    int err=0;   //err为0代表无错误
    clock_t t1,t2;
    Node nodes[2*MAX];        //存放节点和固定的叶子(共511个)，另加一个无用节点
    Queue queues[MAX];          //存放用来排序的节点和叶子序号和权重
    QString filename=getFilename(sfile);  //由路径获取文件名

    t1=clock();
    addInformation("正在构建哈弗曼树...");

    initNodes(nodes);
    initQueues(queues);
    if(!getStatisticsOfCharacters(sf,queues))//步骤：统计字符频率
    {
        addInformation("操作已停止！");
        sfile="";
        sf.close();
        return;
    }
    unsigned int size1;
    QString fileSize=turnFileSize_toQString(size1=getFileSize(sf));
    short int root=createHuffmanTree(nodes,queues);//步骤：创建哈弗曼树
    map <char,string> stringMap;
    createCharacterEncodingTable(stringMap,root,nodes);//步骤：创建字符编码对照表

    addInformation("正在产生编码文件...");
    QTextCodec *code = QTextCodec::codecForName("gb18030");
    std::string route = code->fromUnicode(sfile).data();

    err+=!builtCodeFile(sf,route+".hcode",stringMap,sfile+".hcode");//步骤：生成编码文件
    if(err)
    {
        addInformation("编码失败！");
        sfile="";
        sf.close();
        return;
    }
    QString st;
    fstream ft;
    sfile.remove(sfile.length()-filename.length(),filename.length());
    st=sfile+filename+".hcode";
    code = QTextCodec::codecForName("gb18030");
    ft.open(code->fromUnicode(st).data());
    unsigned int size2=getFileSize(ft);
    err+=!saveHuffmanKey(route,nodes,size2);  //返回值表示打开输出文件是否成功    //步骤：保存哈弗曼树
    t2=clock();
    if(!err)
        QMessageBox::information(this, "提示", "文件压缩完毕！", QMessageBox::Ok);
    else
        {
        QMessageBox::warning(this, "错误", "保存key文件失败！", QMessageBox::Ok);
        sfile="";
        sf.close();
        ft.close();
        return;
        }
    /*-------------------------------------------------------------*/

    addInformation("");
    addInformation("保存编码文件："+st);

    st=turnFileSize_toQString(size2);
    addInformation("编码文件大小："+st);

    st=sfile+filename+".hkey";
    addInformation("保存译码本："+st);
    ft.close();
    code = QTextCodec::codecForName("gb18030");
    ft.open(code->fromUnicode(st).data());
    unsigned int sizeT;
    st=turnFileSize_toQString(sizeT=getFileSize(ft));
    addInformation("译码本大小："+st);
    st=turnFileSize_toQString(size2+sizeT);
    addInformation("总大小："+st);
    addInformation("----------------------------------------------------");
    st=QString::number((float)size2/size1*100,'f',2);
    addInformation("压缩率："+st+"%");
    QString time_s=QString::number((float)(t2-t1)/1000);
    addInformation("用时："+time_s+"秒");
    addInformation("****************************************************");
    ft.close();
    sf.close();
    sfile="";
    /*-------------------------------------------------------------*/
}


void MainWindow::on_pushButton_2_clicked()
{
    if(tag1&&tag2)
    {
        unsigned int codeFileSizeANDjudge;
        QTextCodec *code = QTextCodec::codecForName("gb18030");
        fstream ft;
        ft.open(code->fromUnicode((QString)route2.data()).data());
        codeFileSizeANDjudge=getFileSize(ft);    //获取编码文件大小，用来与译码本中的大小信息做对比

        Node nodes[2*MAX];        //存放节点和固定的叶子(共511个)，另加一个无用节点

        short int root;

        if(!loadHuffmanKey(code->fromUnicode(route1.data()).data(),nodes,root,codeFileSizeANDjudge))//步骤：加载哈弗曼树
        {
            if(codeFileSizeANDjudge==0)
            {
                //QMessageBox::warningm(NULL, "警告！", "此译码本与编码文件不匹配！", QMessageBox::Default, QMessageBox::Default);
                QMessageBox msg_box;
                msg_box.setIcon(QMessageBox::Warning);
                msg_box.setWindowTitle("警告！");
                msg_box.setText("此译码本与编码文件不匹配！还要继续解码此文件？");
                msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msg_box.setDefaultButton(QMessageBox::No);
                if(QMessageBox::No ==msg_box.exec())
                    return;

                //QMessageBox msg(NULL, "", "", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            }
            else
            {
                QMessageBox::warning(this, "错误", "译码本文件打开失败", QMessageBox::Ok);
                return;
            }
        }



        fstream code_load;
        if(!open_CodeFile(code_load,code->fromUnicode(route2.data()).data()))      //步骤：打开编码文件
        {
            QMessageBox::warning(this, "错误", "编码文件打开失败", QMessageBox::Ok);
            return;
        }

        QString qstemp=route2.data();

        string s=getFilename(qstemp).toStdString();
        string directory_path = route2.substr(0, route2.length()-s.length());//s获取文件目录的路径
        string filename=s.substr(0,s.length()-6);
        s=directory_path+decodePrifix.toStdString()+filename;
        addInformation("");
        addInformation("正在解码...");
        clock_t t1,t2;
        t1=clock();
        if(!decodeFile(code_load,code->fromUnicode(s.data()).data(),nodes,root,(QString)s.data())) //步骤：解码文件
        {
            addInformation("解码被取消!");
            addInformation("****************************************************");
            return;
        }

        t2=clock();

        ui->plainTextEdit->setPlainText("");


        addInformation("解码后的文件存放至："+(QString)s.data());
        addInformation("----------------------------------------------------");
        code_load.close();
        code_load.open(code->fromUnicode(s.data()).data());
        qstemp=turnFileSize_toQString(getFileSize(code_load));
        addInformation("文件大小："+qstemp);
        addInformation("用时："+QString::number((float)(t2-t1)/1000,'f',3)+"秒");
        addInformation("****************************************************");
        ft.close();
        code_load.close();
        QMessageBox::information(this, "提示", "解码完成！", QMessageBox::Ok);

    }
    else
        QMessageBox::information(this, "提示", "未选择译码文件或译码本，请先选择文件", QMessageBox::Ok);


}
void MainWindow::on_pushButton_4_clicked(int control)
{
    QString txtfile;
    if(control==0)
    txtfile = QFileDialog::getOpenFileName(this,"Open file",QDir::homePath(),"(*.hcode)");
    else
        txtfile=route2.data();
    if(txtfile.isEmpty())
        return;
    QTextCodec *code = QTextCodec::codecForName("gb18030");
    tag1=1;
    route2=txtfile.toStdString();
    fstream ft;
    QString st;
    addInformation("当前打开的编码文件："+txtfile);

    ft.open(code->fromUnicode(txtfile).data());
    unsigned int fsize=getFileSize(ft);
    st=turnFileSize_toQString(fsize);
    addInformation("编码文件大小："+st);

    if(ui->checkBox->isChecked())    //自动搜寻功能
    {
        QString path=route2.data();
        unsigned int size=getFilename(path).size();
        path.remove(path.size()-size,size);

        unsigned int codeFileSize_in;


        if(!ui->checkBox_2->isChecked())    //当前目录搜寻模式
            path=QFileDialog::getExistingDirectory(NULL, tr("编码文件自动搜寻——请选择文件夹"),path,QFileDialog::ShowDirsOnly);
        addInformation("正在搜寻与此编码文件相匹配的译码本...");
        addInformation("搜寻目录："+path);
        QDir *dir=new QDir(path);
        QStringList filter;
        filter<<"*.hkey";
        dir->setNameFilters(filter);
        QList<QFileInfo> *fileInfo=new QList<QFileInfo>(dir->entryInfoList(filter));
        if(ui->checkBox_3->isChecked())
            addInformation("搜寻详情（找到的译码本文件）：");
        for(int i=0;i<fileInfo->size();i++)
        {
            ft.close();
            ft.open(code->fromUnicode(fileInfo->at(i).filePath()).data());
            ft.read((char *)&codeFileSize_in,4);
            if(ui->checkBox_3->isChecked())
                addInformation(fileInfo->at(i).filePath()+"  "+turnFileSize_toQString(getFileSize(ft)));

            if(fsize==codeFileSize_in)  //若编码文件大小与从译码本读入的编码文件大小相等
            {
                addInformation("");
                tag2=1;
                route1=fileInfo->at(i).filePath().toStdString();
                addInformation("找到译码本："+fileInfo->at(i).filePath());
                addInformation("----------------------------------------------------");
                ft.close();
                return;
            }
        }
        QMessageBox::warning(this,"自动搜索","没有找到译码本文件！",QMessageBox::Ok);
        addInformation("没有找到！");
        addInformation("----------------------------------------------------");
    }
    ft.close();
}

void MainWindow::on_pushButton_3_clicked(int control)
{
    QString txtfile;
    if(control==0)
    txtfile = QFileDialog::getOpenFileName(this,"Open file",QDir::homePath(),"(*.hkey)");
    else
        txtfile=route1.data();
    if(txtfile.isEmpty())
        return;
    QTextCodec *code = QTextCodec::codecForName("gb18030");
    route1 = txtfile.toStdString();
    tag2=1;
    fstream ft;
    QString st;
    addInformation("当前打开的译码文件："+txtfile);

    ft.open(code->fromUnicode(txtfile).data());
    if(!ft.is_open())
    {
        QMessageBox::warning(this,"错误","译码本不能打开",QMessageBox::Ok);
        addInformation("打开译码本错误!");
        return;
    }
    st=turnFileSize_toQString(getFileSize(ft));
    addInformation("译码本大小："+st);


    if(ui->checkBox->isChecked())    //自动搜寻功能
    {
        QString path=route1.data();
        unsigned int size=getFilename(path).size();
        path.remove(path.size()-size,size);

        unsigned int codeFileSize_in;
        ft.read((char *)&codeFileSize_in,4);
        if(ui->checkBox_3->isChecked())
            addInformation("匹配大小："+turnFileSize_toQString(codeFileSize_in));

        if(!ui->checkBox_2->isChecked())    //当前目录搜寻模式
            path=QFileDialog::getExistingDirectory(NULL, tr("编码文件自动搜寻——请选择文件夹"),path,QFileDialog::ShowDirsOnly);
        addInformation("正在搜寻与此译码本相匹配的编码文件...");
        addInformation("搜寻目录："+path);
        QDir *dir=new QDir(path);
        QStringList filter;
        filter<<"*.hcode";
        dir->setNameFilters(filter);
        QList<QFileInfo> *fileInfo=new QList<QFileInfo>(dir->entryInfoList(filter));
        if(ui->checkBox_3->isChecked())
            addInformation("搜寻详情（找到的编码文件）：");
        for(int i=0;i<fileInfo->size();i++)
        {
            ft.close();
            ft.open(code->fromUnicode(fileInfo->at(i).filePath()).data());
            size=getFileSize(ft);
            if(ui->checkBox_3->isChecked())
                addInformation(fileInfo->at(i).filePath()+"  "+turnFileSize_toQString(getFileSize(ft)));

            if(size==codeFileSize_in)  //若编码文件大小与从译码本读入的编码文件大小相等
            {
                addInformation("");
                tag1=1;
                route2=fileInfo->at(i).filePath().toStdString();
                addInformation("找到编码文件："+fileInfo->at(i).filePath());
                addInformation("----------------------------------------------------");
                ft.close();
                return;
            }
        }
        QMessageBox::warning(this,"自动搜索","没有找到编码文件！",QMessageBox::Ok);
        addInformation("没有找到！");
        addInformation("----------------------------------------------------");
    }
    ft.close();
}

void MainWindow::on_plainTextEdit_textChanged()
{
}

void MainWindow::on_pushButton_5_clicked()
{
    ui->plainTextEdit->setPlainText("");
    information="";
}

void MainWindow::on_action_2_triggered()
{
         on_pushButton_open_clicked();
}

void MainWindow::on_action_3_triggered()
{
    on_pushButton_4_clicked();
}

void MainWindow::on_action_4_triggered()
{
    on_pushButton_3_clicked();
}

void MainWindow::on_action_triggered()
{
    ui->plainTextEdit->setPlainText("");
    information="";
}


void MainWindow::on_action_8_triggered()
{
    QMessageBox::about(this, "关于", "哈弗曼压缩2.4\ncreate by ss\n2013.12.24");

}

void MainWindow::on_action_7_triggered()
{
   QMessageBox::about(this, "帮助", "特性说明：\n此程序能根据哈弗曼算法压缩和解压缩单个文件，\n文件无类型和大小限制。支持中文路径。\n能自动判别译码本和编码文件是否匹配。\n有完整可靠的错误处理机制，确保用户在极端情况下使用也不会崩溃。\n新增文件自动搜寻功能，只需一步即可解压文件!\n新增文件拖拽功能，简单易用");

}


void MainWindow::on_checkBox_clicked(bool checked)
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

void MainWindow::on_pushButton_open_clicked(int control)
{
    QString  st;
    if(control==0)
        st = QFileDialog::getOpenFileName(this,"打开原文件",QDir::homePath());
    else
        st=sfile;
    if(st=="")
        return;
    sf.close();
    sfile=st;
    QString filename=getFilename(sfile);  //由路径获取文件名
    QTextCodec *code = QTextCodec::codecForName("gb18030");
    std::string route = code->fromUnicode(sfile).data();
    /*-------------------------------------------------------------*/

    int err=0;
    information="";
    addInformation("打开原文件："+sfile);
    err+=!open_file(sf,route); //返回值表示是否打开成功
    if(err)
    {
        addInformation("失败！");
        return;
    }
    tag1=tag2=0;
    unsigned int size1;
    QString fileSize=turnFileSize_toQString(size1=getFileSize(sf));
    ui->plainTextEdit->setPlainText("");
    addInformation("文件名："+filename);
    addInformation("文件大小："+fileSize);
}

void MainWindow::on_pushButton_key_clicked()
{

    QString txtfile;
    txtfile=route1.data();
    if(txtfile.isEmpty())
    {
        QMessageBox::information(this,"提示","没有选择译码本",QMessageBox::Ok);
        return;
    }
    fstream ft;
    QString st;

    QTextCodec *code = QTextCodec::codecForName("gb18030");
    ft.open(code->fromUnicode(txtfile).data());
    if(!ft.is_open())
    {
        QMessageBox::warning(this,"错误","译码本不能打开",QMessageBox::Ok);
        addInformation("打开译码本错误!");
        return;
    }
    ui->pushButton_key->setEnabled(false);
    saveInformation.push(information);
    information="";
    unsigned int codeFileSize;
    ft.read((char *)&codeFileSize,4);

    st=turnFileSize_toQString(getFileSize(ft));
    addInformation("当前打开的译码文件："+txtfile);
    addInformation("译码本大小："+st);
    addInformation("");
    addInformation("匹配信息（编码文件大小）："+turnFileSize_toQString((float)codeFileSize));
    addInformation("字节数："+st.setNum(codeFileSize));
    addInformation("");

    Node nodes[2*MAX];
    short int root;
    if(!loadHuffmanKey(code->fromUnicode(txtfile).data(),nodes,root,codeFileSize))
    {
        ui->pushButton_key->setEnabled(true);
        addInformation("加载hkey文件失败!");
        return;
    }

    addInformation("树信息：");
    addInformation("节点信息\t序号\t左孩子\t右孩子\t双亲");
    QString s;
    for(int i=0;i<511;i++)
        if(nodes[i].parent<511)
        {
            s="";
            if(i<256)
            {
                st="[叶]";
                if(i==10)
                    s+=st+"换行";
                else if(i==13)
                    s+=st+"回车";
                else
                s+=st+(char)i;
            }
            else
                if(root==i)
                    s.append("[根]");
                else
                    s.append("[节]");
            st.setNum(i);
            s+="\t"+st;
            st.setNum(nodes[i].lchild);
            s+="\t"+st;
            st.setNum(nodes[i].rchild);
            s+="\t"+st;
            st.setNum(nodes[i].parent);
            s+="\t"+st;
            addInformation(s);
        }

    ui->pushButton_key->setEnabled(true);
}

void MainWindow::on_pushButton_key_2_clicked()
{
    if(!saveInformation.isEmpty())
    information=saveInformation.pop();
    ui->plainTextEdit->setPlainText(information);
}

void MainWindow::on_action_10_triggered()
{
    setDialog dialog;
    bool check1=ui->checkBox->isChecked();
    bool check2=ui->checkBox_2->isChecked();
    bool check3=ui->checkBox_3->isChecked();
    dialog.inMessage(decodePrifix,check1,check2,check3,c_check1);

    dialog.show();
    dialog.setFixedSize(dialog.size());


    dialog.exec();
    dialog.outMessage(decodePrifix,check1,check2,check3,c_check1);

    ui->checkBox->setChecked(check1);
    ui->checkBox_2->setChecked(check2);
    ui->checkBox_3->setChecked(check3);
    on_checkBox_clicked(check1);
    ui->frame->setVisible(c_check1);
}
