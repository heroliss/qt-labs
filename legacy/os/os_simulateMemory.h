//os_simulateMemory.h
#ifndef OS_SIMULATEMEMORY
#define OS_SIMULATEMEMORY

#include "os_memoryError.h" //自定义异常类
#include "os_typeDefine.h"  //自定义类型

class SimulateMemory // 模拟内存
{
private:
    int BLOCK_SIZE;//块大小,单位：字节
    int blockNum; //内存大小，单位：块数
    int freeBlockNum;//空闲块数量
	char *blockList; //模拟内存,<块内容>,标号为块地址
	
public:
    SimulateMemory(int block_num,int blockSize);
    void spaceOccupy(int block_id,char content); //占用内存动作
    void spaceFree(int block_id);//释放内存
    char getMark(int block_id){return blockList[block_id];}
    char at(int block_id){return blockList[block_id];} //此函数与上一个函数相同
    int getMemorySize(){return blockNum*BLOCK_SIZE;}
    int getBlockNum(){return blockNum;}
    int getBlockSize(){return BLOCK_SIZE;}
    int getFreeSpaceSize(){return freeBlockNum*BLOCK_SIZE;}
    int getFreeBlockNum(){return freeBlockNum;}
    int getOccupiedSpaceSize(){return (blockNum-freeBlockNum)*BLOCK_SIZE;}
    int getOccupiedBlockNum(){return blockNum - freeBlockNum;}
    void moveContentForward(int begin,int size,int n);//将一区域块内的内容向前移动n个块
};

#endif
