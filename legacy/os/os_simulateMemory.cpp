#include "os_simulateMemory.h"

SimulateMemory::SimulateMemory(int block_num,int blockSize)
{

    BLOCK_SIZE=blockSize; //传入块大小
    blockNum=block_num;   //传入块数量

    blockList=new char[blockNum];//构建虚拟内存，内容全部为0
    for (int i=0;i<blockNum;i++)
        blockList[i]=0;

    freeBlockNum =blockNum; //初始化空闲块数量
}
void SimulateMemory::moveContentForward(int begin,int size,int n)
{
    char t;
    int block_id;
    for (block_id=begin;block_id<begin+size;block_id++)
    {
        t=blockList[block_id];
        blockList[block_id]=0;
        blockList[block_id-n]=t;

    }
}
void SimulateMemory::spaceOccupy(int block_id,char content)
{
    blockList[block_id]=content;  //模拟内存写入数据
    freeBlockNum--;

}
void SimulateMemory::spaceFree(int block_id)
{
    blockList[block_id]=0;           //模拟内存释放
    freeBlockNum++;
}
