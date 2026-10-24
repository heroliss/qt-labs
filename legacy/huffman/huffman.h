#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <iostream>
#include <algorithm>
#include <map>
#include <fstream>
#include<QProgressDialog>
#include<QFile>
#include<QMessageBox>

#define MAX 256
using namespace std;
/***********************************/
typedef struct node
{
    unsigned short int lchild;
    unsigned short int rchild;
    unsigned short int parent;
}Node;
/**********************队列结构体***************************/
typedef struct queue
{
    unsigned short int ch;
    unsigned frequency;
}Queue;
/********************比较规则*************************/
bool cmp(const Queue &a,const Queue &b)         //比较规则，按频率由大到小
{
    return a.frequency>b.frequency;
}
/***********************打开源文件*****************************/
bool open_file(fstream &f1,const string &route)
{
    f1.open(route.data(),ios::in|ios::binary);
    if (!f1.good())
    {
        f1.clear();
        return false;
    }
    return true;
}
/************************初始化节点表***************************/
void initNodes(Node *nodes)
{
    unsigned int i;
    for (i=0;i<2*MAX;i++)
    {
        nodes[i].parent=2*MAX-1;   //初始化节点表中的双亲为最后一个无用节点
    }
}
/************************初始化队列表***************************/
void initQueues(Queue *queues)
{
    unsigned int i;
    for (i=0;i<MAX;i++)            //初始化队列表
    {
        queues[i].frequency=0;     //用0初始化节点频率后不能作为有效节点，否则俩节点频率之和仍为0,
        queues[i].ch=i;
    }
}
/************************获取文件大小***************************/
int getFileSize(fstream &f1)
{
    f1.seekg(0,ios::end);
    streampos ps=f1.tellg();    //获取文件大小
    f1.seekg(0,ios::beg);
    return ps;
}
/*********************统计字符频率******************************/
bool getStatisticsOfCharacters(fstream &f1,Queue *queues)
{
    int processNum=0;
    QProgressDialog process;

    process.setLabelText("正在构建哈弗曼树...");
    process.setRange(0,getFileSize(f1));
    process.setModal(true);
    process.setCancelButtonText("取消");
    process.setValue(processNum);//用来指示压缩进度
    int processnum=getFileSize(f1)/100,tag=0;
    char ch;
    while (f1.peek()!=EOF)                //队列表中统计字符出现的次数
    {
        f1.get(ch);
        //if (f1.fail())break;
        queues[(unsigned char)ch].frequency++;  //此处必须对字符进行强制类型转换，否则汉字所占的其中一个字符的数字会是负数，而负数在节点表里没有对应值
        tag++;
        processNum++;
        if(tag==processnum)
        {
            tag=0;
        process.setValue(processNum);//用来指示压缩进度
        }
        if(process.wasCanceled())
        {
           QMessageBox::information(NULL, "提示", "构建哈弗曼树已取消！", QMessageBox::Default | QMessageBox::Default);
           return false;
        }
    }
    return true;
}

/*********************反复排序构建哈弗曼树**********************/
short int createHuffmanTree(Node *nodes,Queue *queues)     //返回根节点位置
{
/***首次排序并设置队列头尾***/

    int tail=MAX-1;  //队尾位置
    unsigned short int n=MAX;    //节点下标

    sort(queues,queues+tail+1,cmp);//为队列排序，最小值放在最后
    while (queues[tail].frequency==0)  //tail指向前移至有效节点处
        tail--;

    if (tail==0)  //如果源文件中只有一种类型的字符，为避免错误需强行设置俩个叶子节点
        tail++;

/***反复排序构建哈弗***/
    while (tail>0)
    {
        sort(queues,queues+tail+1,cmp);//为队列排序，最小值放在最后

        nodes[n].lchild=queues[tail].ch;                 //尾部的俩个叶子信息赋值到节点表中
        nodes[n].rchild=queues[tail-1].ch;
        //nodes[n].parent=NULL; //取消为每个节点初始化双亲指向

        nodes[queues[tail].ch].parent=n;                 //为俩个叶子设置双亲
        nodes[queues[tail-1].ch].parent=n;

        queues[tail-1].ch=n;                             //在队列尾部添加新增的节点
        queues[tail-1].frequency+=queues[tail].frequency; //频率相加
        tail--;            //移除一个叶子或节点
        n++;               //指向下一个新增节点的位置

    }
    short root=n-1; //根节点指向
    nodes[root].parent=root;
    return root;
}
/*********************生成字符编码对照表************************/
void createCharacterEncodingTable(map<char,string> &stringMap,short int root,Node *nodes)
{
    string stringCode,stringCode_reverse;
    int j;
    unsigned short int n,c;
    for (c=0;c<MAX;c++)
    {
        stringCode="";
        stringCode_reverse="";
        n=c;

        if (nodes[c].parent<2*MAX-1) //保证此节点为有效节点
        {
            while (n<root)
            {

                if (nodes[nodes[n].parent].lchild==n)
                    stringCode+=(char)0;
                else
                    stringCode+=(char)1;
                n=nodes[n].parent;

            }
            for(j=stringCode.length()-1;j>=0;j--)
                stringCode_reverse+=stringCode[j];
            stringMap.insert(pair<char,string>(c,stringCode_reverse));
        }
    }
}
/***********************合成文件编码****************************/
bool builtCodeFile(fstream &f1,const string &route,map<char,string> &stringMap,const QString Qroute)
{
    /*遍历文件*/
    string filename=route;
    fstream f2;

    char bit;
    char ch;
    char j=7;
    unsigned int i;

    f2.open(filename.data(),ios::binary|ios::out);
    if (f2.bad())
    {
        f2.close();
        return false;
    }
    f1.clear();//移动读指针前需用此函数清除文件的eof状态
    f1.seekg(0,ios::beg);
    string wordcode;

    int processNum=0;
    QProgressDialog process;

    process.setLabelText("编码中...");
    process.setRange(0,getFileSize(f1));
    process.setModal(true);
    process.setCancelButtonText("取消");

    process.setValue(processNum);//用来指示压缩进度
    int processnum=getFileSize(f1)/100,tag=0;
    unsigned int len;
    while (f1.peek()!=EOF)      //此处eof到文件末尾不会立即返1，因此导致文件尾部的字符多读取一次！
    {

        f1.get(ch);
        //if (f1.fail())break; //此句用来解决上述问题，经测试对编码性能影响非常小 (因上面使用peek函数，故此句省略)
        i=0;
        wordcode=stringMap[ch];
        len=wordcode.length();
        while ( i<len&&j>=0)
        {

            bit+=wordcode.at(i);
            if (j==0)
            {

                f2.write(&bit,1);
                j=7;
                bit=0;
            }else
            {
                j--;
                bit<<=1;
            }

            i++;
        }
        tag++;
        processNum++;
        if(tag==processnum)
        {
            tag=0;
            process.setValue(processNum);//用来指示压缩进度
        }
        if(process.wasCanceled())
        {
            f2.close();
            if(QFile::remove(Qroute))
                QMessageBox::information(NULL, "提示", "编码已取消！残余的编码文件已删除！", QMessageBox::Default | QMessageBox::Default);
            else
                QMessageBox::information(NULL, "提示", "编码已取消！残余的编码文件删除失败！请手动删除该残余文件：\n"+Qroute, QMessageBox::Default | QMessageBox::Default);
            return false;
        }

    }
    //若最后一组编码不足8位，则写入最后一个字符和一个有效位数指示数
    //charCompress=bit.to_ulong();
    bit<<=j;      //不足8位的数据添加到了bit字符右侧，需要用此句来移位到左侧
    f2.write(&bit,1);
    f2<<(char)(7-j);
    f2.close();
    return true;
}
/************************保存哈弗曼树***************************/
typedef struct node_save  //保存至文件的节点结构
{
    unsigned short int index;
    unsigned short int lchild;
    unsigned short int rchild;
    unsigned short int parent;
}Node_save;
bool saveHuffmanKey(const string &route,Node *nodes,const unsigned int &codeFileSize)
{
    int i;
    fstream file;
    string filename=route+".hkey";
    file.open(filename.data(),ios::binary|ios::out);
    if (file.bad())
    {
        file.close();
        return false;
    }
    file.write((const char*)&codeFileSize,4);
    for (i=0;i<2*MAX-1;i++)
    {
        if (nodes[i].parent<2*MAX-1)
        {
            Node_save *nodeSave=new Node_save;
            nodeSave->index=i;
            nodeSave->lchild=nodes[i].lchild;
            nodeSave->rchild=nodes[i].rchild;
            nodeSave->parent=nodes[i].parent;
            file.write((const char*)nodeSave,sizeof(Node_save));
        }
    }
    file.close();
    return true;
}
/************************载入哈弗曼树**************************/
bool loadHuffmanKey(const string &route,Node *nodes,short int &root,unsigned int &codeFileSizeANDjudge)
{
    int i;
    for (i=0;i<2*MAX;i++)
    {
        nodes[i].parent=2*MAX-1;   //初始化节点表中的双亲为最后一个无用节点
    }

    Node_save node_read;
    string filename=route;
    fstream ftree;
    ftree.open(filename.data(),ios::binary|ios::in);
    if (ftree.bad())
    {
        ftree.close();
        return false;
    }
    unsigned int codeFileSize_in;
    ftree.read((char *)&codeFileSize_in,4);
    if(codeFileSize_in!=codeFileSizeANDjudge)
    {
        codeFileSizeANDjudge=0;   //匹配错误 若匹配正确不改变原参数的值
        return false;
    }

    while (!ftree.eof())
    {
        ftree.read((char*)&node_read,sizeof(Node_save));
        nodes[node_read.index].lchild=node_read.lchild;
        nodes[node_read.index].rchild=node_read.rchild;
        nodes[node_read.index].parent=node_read.parent;
    }
    root=node_read.index;  //最后读取进来的节点是根节点
    ftree.close();
    return true;
}
/************************打开编码文件**************************/
bool open_CodeFile(fstream &code_load,const string &route)
{
    string filename=route;
    code_load.open(filename.data(),ios::binary|ios::in);
    if (code_load.bad())
    {
        code_load.close();
        return false;
    }
    return true;
}
/**************************解码文件****************************/
QString getFilename(const QString &s)   //获取文件名
{
    QString rs;
    int i;
    i=s.length();
    i--;
    while(s.at(i)!='/'&&i>=0)
    {
        rs.insert(0,s.at(i));
        i--;
    }
    return rs;
}
QString getFileType(const QString &s)   //获取文件类型
{
    QString rs;
    int i;
    i=s.length();
    i--;
    while(s.at(i)!='.'&&i>=0)
    {
        if(s.at(i)=='/')
            return rs="";
        rs.insert(0,s.at(i));
        i--;
    }
    return rs;
}
bool decodeFile(fstream &code_load,const string &route,Node *nodes,short int root,const QString &Qroute)
{
    int j;

    string filename=route;
    fstream fsave;
    fsave.open(filename.data(),ios::out|ios::binary);
    if (fsave.bad())
    {
        fsave.close();
        return false;
    }
    char ch;
    unsigned short n=root;
    unsigned char m=1;   //m中存放单个位的1，来检测读入字符的对应位是1还是0.此处必须用unsigned char类型，否则>>指令会是算术右移指令，而我们需要逻辑右移
    code_load.seekg(-1,ios::end);     //文件至少有2个字符
    streamoff fp=code_load.tellg();   //记录倒数第二个字节的在文件中的地址
    streamoff currentFp=1;
    char endMessage,endCode;
    code_load.seekg(-2,ios::end);
    code_load.get(endCode);
    code_load.get(endMessage);
    code_load.clear();
    code_load.seekg(0,ios::beg);

    int processNum=0;
    QProgressDialog process;
    process.setLabelText("解码中...");
    process.setRange(0,getFileSize(code_load));
    process.setModal(true);
    process.setCancelButtonText("取消");
    process.setValue(processNum);//用来指示压缩进度
    int processnum=getFileSize(code_load)/100,tag=0;

    while (1)
    {
        m=128;

        code_load.get(ch);
        if (fp!=currentFp++)
        {

            for (j=7;j>=0;j--)
            {
                if (ch&m)
                {
                    n=nodes[n].rchild;
                }else /*if (code.at(i)=='0')*/
                {
                    n=nodes[n].lchild;
                }
                if (n<MAX)  //此时nodes[n]必为叶子
                {
                    fsave.put((char)n);
                    n=root;
                }
                m>>=1;
            }
        }else
        {
            for (j=7;j>=8-endMessage;j--)  //若endmessage为0则不读取最后一个数据字节的内容
            {
                if (ch&m)
                {
                    n=nodes[n].rchild;
                }else /*if (code.at(i)=='0')*/
                {
                    n=nodes[n].lchild;
                }
                if (n<MAX)  //此时nodes[n]必为叶子
                {
                    fsave.put((char)n);
                    n=root;
                }
                m>>=1;
            }
            break;
        }

        tag++;
        processNum++;
        if(tag==processnum)
        {
            tag=0;
        process.setValue(processNum);//用来指示压缩进度
        }
        if(process.wasCanceled())
        {
                fsave.close();
                if(QFile::remove(Qroute))
                QMessageBox::information(NULL, "提示", "解码已取消！残余的解码文件已删除！", QMessageBox::Default | QMessageBox::Default);
                else
                QMessageBox::information(NULL, "提示", "解码已取消！残余的解码文件删除失败！请手动删除该文件：\n"+Qroute, QMessageBox::Default | QMessageBox::Default);

                return false;
        }
    }
    fsave.close();
    return true;
}

/*************************************/

#endif // HUFFMAN_H
