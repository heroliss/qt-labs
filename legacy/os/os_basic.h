#ifndef OS_BASIC
#define OS_BASIC
#include "os_typedefine.h"
#include "os_simulateMemory.h"

#include <QString>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <vector>
using namespace std;

class OS_Basic //基础操作系统类
{
private:
    int freeSpace;//最大空闲空间大小
	SimulateMemory *memory;//虚拟内存
	
    map<int,PCB*> *pcbList;//按进程首地址排序的pcb指针表
    vector<pair<int,int> > *freeSpaceList;//空闲空间列表
    vector<PCB*> ready_IndexList;//PCB就绪表
	vector<PCB*> block_IndexList;//PCB阻塞表
    vector<PCB*> run_IndexList;  //PCB运行表
	
    void writeMemory(int insert_address,int pages_amount,char content);//整段写入内存(内存地址，段大小，内容)
    void freeMemory(int begin,int block_amount);//整段释放内存
    int getInsertAddress(int pages_amount);//获取合适的内存地址
	
	void scheduleRunningProcess();//就绪态调度到执行态(删除程序时)
    void recoverMemory(int begin,int size);//在空闲空间表里回收内存
	
public:
    OS_Basic(int memoryBlockNum,int memoryBlockSize); //参数不能为0
    void createProcess(QString name, int pages_amount, char content);
	void destroyRunningProcess(); //结束正在运行的程序 
	
	void compactMemory();//内存紧凑	
	
	bool runToBlock();//执行态调度到阻塞态
	bool runToReady();
	bool readyToRun();
	bool blockToReady(int id);
	void timeRoundRobin(){runToReady();readyToRun();}//时间片轮转(一次)
	
	SimulateMemory *getMemory(){return memory;}	
    map<int,PCB*> *getPCBlist(){return pcbList;}//按内存顺序排列的pcb表
    vector<pair<int,int> > *getFreeSpaceList(){return freeSpaceList;}//空闲空间列表
    vector<PCB*>* getReadyList(){vector<PCB*> *p=new vector<PCB*> (ready_IndexList);return p;}//PCB就绪表
	vector<PCB*>* getBlockList(){vector<PCB*> *p=new vector<PCB*> (block_IndexList);return p;}//PCB阻塞表
    vector<PCB*>* getRunIndexList(){vector<PCB*> *p=new vector<PCB*> (run_IndexList);return p;}//PCB运行表
	
    bool memoryFull(){return freeSpace==0;}
    bool memoryEmpty(){return freeSpace==memory->getFreeBlockNum();}
};

/******************************************************************************************************/
#endif
