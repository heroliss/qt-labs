//os_typeDefine.h
#ifndef OS_TYPEDEFINE
#define OS_TYPEDEFINE
#include <QString>


enum STATE {Run,Ready,Blocked};
enum PRIORITY {Lowest,Low,Middle,High,Highest};
typedef struct pageMessage
{
	bool state; //状态
    int blockID;//对应的存储块
	
}PAGE;
typedef struct segmentMessage
{
	bool state;
    int pageSize;//页表大小
	PAGE *pageAddress;//页表首地址
}SEGMENT;

typedef struct blockMessage   //虚拟的内存单位
{
	char blockMark;   //颜色标号 ,即虚拟的内存内容
}BLOCK;

/******************************************************/
typedef struct pcb_struct  //定义进程控制块PCB
{
	/*进程标识符*/
    //int id; //内部标识符
    QString name;     //外部标识符

	/*处理机状态*/
		//通用寄存器
		//指令计数器
		//程序状态字PSW
		//用户栈指针

	/*进程调度信息*/
	STATE state;            //进程状态
	PRIORITY priority;      //进程优先级
    //int time_run;  //进程执行时间
    //int time_wait; //进程已等待cpu的时间总和
	//int reason_runToBolck;  //进程阻塞所等待发生的事件
	
	/*进程控制信息*/
		//进程同步和通信机制
		//资源清单
    int segment_address;   //段首地址
    int pages_amount;      //页总数

	//struct pcb_struct *next;    //链接指针,指向下一个进程的 段首地址


}PCB;

#endif

