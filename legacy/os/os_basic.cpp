#include "os_basic.h"
#include "limits"
bool cmp( pair<int,int> p1,pair<int,int>  p2){return p1.second > p2.second;}//空闲空间由大到小排序
/******************************************************************************************************/
OS_Basic::OS_Basic(int memoryBlockNum,int memoryBlockSize)
{
    freeSpaceList=new vector<pair<int,int> >;
    memory=new SimulateMemory(memoryBlockNum,memoryBlockSize);
    freeSpaceList->push_back(make_pair(0,memoryBlockNum));
    freeSpace=memoryBlockNum;
    pcbList=new map<int,PCB*>();

}
/******************************************************************************************************/
void OS_Basic::recoverMemory(int begin,int size)
{
    vector<pair<int,int> >::iterator it;
    vector<pair<int,int> >::iterator itUp=freeSpaceList->end();
    vector<pair<int,int> >::iterator itDown=freeSpaceList->end();
    int closestUp=0;//前方最近空闲空间首地址(初始化为最小值)
    int closestDown=INT_MAX;

    for (it=freeSpaceList->begin();it!=freeSpaceList->end();it++)
    {
        if (closestUp<=it->first && it->first <=begin)
        {
            closestUp=it->first;
            itUp=it;
        }
    }

    for (it=freeSpaceList->begin();it!=freeSpaceList->end();it++)
    {
        if (closestDown>it->first && it->first >=begin+size)
        {
            closestDown=it->first;
            itDown=it;
        }
    }
    if (itUp==freeSpaceList->end())
        closestUp=-1;//如果没有前面的空闲空间，就把最近上界修改为一个与回收区首地址不相等的地方
    else
        closestUp+=itUp->second;
    //上面分别得到了距回收区最近的前后俩个空闲空间的末地址和首地址

    if (closestUp==begin&&closestDown!=begin+size)
    {
        itUp->second+=size;
    }else if (closestUp!=begin&&closestDown==begin+size)
    {
        itDown->second+=size;
        itDown->first=begin;
    }else if (closestUp==begin&&closestDown==begin+size)
    {
        itUp->second+=(size+itDown->second);
        freeSpaceList->erase(itDown);
    }else if (closestUp!=begin&&closestDown!=begin+size)
    {
        freeSpaceList->push_back(make_pair(begin,size));
    }
}
/******************************************************************************************************/
void OS_Basic::scheduleRunningProcess()
{
    if (run_IndexList.empty())
    {
        ready_IndexList.front()->state=Run;//设置pcb中状态为运行态
        run_IndexList.push_back(ready_IndexList.front());
        ready_IndexList.erase(ready_IndexList.begin());
    }
}
/******************************************************************************************************/
void OS_Basic::writeMemory(int insert_address,int pages_amount,char content)
{
    int i;
    for (i=insert_address;i<insert_address+pages_amount;i++)
    {
        memory->spaceOccupy(i,content);
    }
}
/******************************************************************************************************/
void OS_Basic::freeMemory(int begin,int block_amount)
{
    int i;
    for (i=begin;i<begin+block_amount;i++)
    {
        memory->spaceFree(i);
    }
}
/******************************************************************************************************/
int OS_Basic::getInsertAddress(int pages_amount)
{
    if (pages_amount>freeSpace)
        throw memory_error("内存不足!");
    sort(freeSpaceList->begin(),freeSpaceList->end(),cmp);//空间大小排序
    if (pages_amount>freeSpaceList->at(0).second)
        compactMemory();//内存紧凑
    return freeSpaceList->at(0).first;//返回第一个空闲内存区的首地址

}

/******************************************************************************************************/
void OS_Basic::createProcess(QString name,int pages_amount,char content)
{
    int insert_address;//进程插入内存的首地址
    try{
        insert_address = getInsertAddress(pages_amount);//通过计算得到合适的内存地址
    }
    catch (memory_error e) {	throw  memory_error("内存不足！");}


    PCB *p=new PCB();
    p->name=name;
    p->pages_amount=pages_amount;
    p->priority=Middle;
    p->segment_address=insert_address;
    p->state=Ready;

    ready_IndexList.push_back(p);//将pcb填入就绪列表中
    scheduleRunningProcess();//调度就绪表到执行表
    (*pcbList)[insert_address]=p;//插入到pcb-map表

    writeMemory(insert_address,pages_amount,content);
    freeSpace=freeSpace-pages_amount;

    if(freeSpaceList->at(0).second == pages_amount)//若占用的空间等于空闲空间，删除该空闲空间
        freeSpaceList->erase(freeSpaceList->begin());
    else
    {
        (*freeSpaceList)[0].first=insert_address+pages_amount;
        (*freeSpaceList)[0].second=freeSpaceList->at(0).second-pages_amount;
    }

}
/******************************************************************************************************/
void OS_Basic::destroyRunningProcess()
{
    if (run_IndexList.empty())
        throw memory_error("没有正在运行的程序！");
    PCB *p=run_IndexList.front();

    freeMemory(p->segment_address,p->pages_amount);//从虚拟内存里删除程序
    freeSpace=freeSpace+p->pages_amount;
    recoverMemory(p->segment_address,p->pages_amount);//在内存空闲表里恢复空闲空间
    pcbList->erase(p->segment_address);
    delete(p);//删除PCB实体

    run_IndexList.erase(run_IndexList.begin());//删除运行态表中的运行项

    if (!ready_IndexList.empty()) //若就绪态列表不为空，提取第一个插入到运行态
    {
        run_IndexList.push_back(ready_IndexList.front());
        ready_IndexList.erase(ready_IndexList.begin());
        run_IndexList.front()->state=Run;//修改pcb中的状态
    }




}
/******************************************************************************************************/
void OS_Basic::compactMemory()
{
    int t=0;//初始化中间地址变量
    int distance,newSegmentAdress;//移动距离
    map<int,PCB*>::iterator it;
    PCB *p;
    map<int,PCB*> *pcbList_new=new map<int,PCB*>();
    for (it=pcbList->begin();it!=pcbList->end();it++)
    {
        distance = it->first - t;
        p=it->second;
        memory->moveContentForward(p->segment_address,p->pages_amount,distance);//内存里的内容前移
        p->segment_address -= distance;//一个pcb中的段首地址前移
        newSegmentAdress=p->segment_address;
        t+=p->pages_amount;//中间地址变量移动至当前已占用空间末端
        (*pcbList_new)[newSegmentAdress]=p;//map新表中插入一个新项

    }
    vector<pair<int ,int> > *freeSpaceList_new=new vector<pair<int ,int> >();
    if (freeSpace!=0)
        freeSpaceList_new->push_back(make_pair(t,freeSpace));
    delete freeSpaceList;
    freeSpaceList=freeSpaceList_new;

    delete pcbList;
    pcbList=pcbList_new;

}
/******************************************************************************************************/
bool OS_Basic:: runToBlock()
{
    if (run_IndexList.empty())
        return false;
    run_IndexList.front()->state=Blocked;
    block_IndexList.push_back(run_IndexList.front());
    run_IndexList.erase(run_IndexList.begin());
    readyToRun();
    return true;
}
bool OS_Basic:: runToReady()
{
    if (run_IndexList.empty())
        return false;
    run_IndexList.front()->state=Ready;
    ready_IndexList.push_back(run_IndexList.front());
    run_IndexList.erase(run_IndexList.begin());
    return true;

}
bool OS_Basic:: readyToRun()
{
    if (ready_IndexList.empty())
        return false;
    ready_IndexList.front()->state=Run;
    run_IndexList.push_back(ready_IndexList.front());
    ready_IndexList.erase(ready_IndexList.begin());
    return true;
}
bool OS_Basic:: blockToReady(int id)
{
    if (block_IndexList.empty())
        return false;
    PCB* p;
    try {p = block_IndexList.at(id);}
    catch (out_of_range e) {return false;}

    p->state=Ready;
    ready_IndexList.push_back(p);
    block_IndexList.erase(block_IndexList.begin()+id);
    if (run_IndexList.empty())
        scheduleRunningProcess();
    return true;
}
