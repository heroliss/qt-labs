//stack.h

/*****************************************************************************************************************/
/* 头文件名：栈模板 
/* 存储结构：链表
/* 创建时间：2013.9.25
/* 更新时间：2013.9.25
/* 函数功能与用法：
/*
/*     构造函数：  Stack(T bottom_date); 必须添加一个不变的栈底元素数据作为参数，可以作为栈的显示名用
/*     压入数据：  void push(T s);       自动申请新元素结构体 
/*     弹出：      T pop();              弹出的数据转存至私有成员s中，原数据及元素被删除,返回s（副本）
/*     清空：      void clear();         删除所有元素和数据实体，仅保留栈底元素和数据
/*     是空栈:     int is_empty();       是空栈返回1，否则返回0
/*	  
/*	   获取栈长：  int return_length();      空栈栈长为0（栈底元素永远存在）
/*     获取栈顶数据实体：T show_top();       与pop类似，但无删除操作和临时保存操作
/*
/*     获取临时保存的数据：T return_s();     用于多次访问s（上一次被弹出的栈顶元素数据实体）
/*	  
/*	   返回栈底元素数据指针：void *return_bottom(); 用于修改栈顶和栈底数据指向（不常用）
/*     返回栈顶元素数据指针：void *return_top();
/*
/* 注意事项：
/*     元素中的数据成员是void *指针类型，有时需要强制类型转换对数据地址寻址才正确                         
/*****************************************************************************************************************/
#ifndef STACK_H
#define STACK_H

/*****元素结构体********/
typedef struct element 
{
	void *date;
	struct element *next;

}Elem;
/************栈类***********/
template<class T>
class Stack
{
public:
	Stack(T bottom_date);     //构造一个空栈(必须添加一个不变的栈底元素数据，可以作为栈的显示名用)
	~Stack(){clear();}   //析构函数
	void clear();     //清空该栈
	int is_empty(){if (top->next==NULL)    //判断是否为空栈
		return 1; else return 0;}  
	int return_length(){return length;}    //返回长度

	
	void *return_bottom(){return bottom->date;}//返回栈底元素指针
	void *return_top(){return top->date;}            //返回栈顶元素指针
	T show_top(){return *(T *)(top->date);} //返回栈顶元素指向的数据实体
	
	void push(T s);          //压入一个数据
	T pop();           //弹出，即删除一个元素并返回临时保存的元素实体（元素的数据实体保存至s）（返回s的副本）
	T return_s(){return s;}      //返回临时保存的数据（用于多次访问s）

private:
	Elem *top;         //栈顶元素地址
	int length;        //元素个数
	Elem *bottom;      //栈底元素地址（不变）
	T s;          //保存栈顶元素的数据实体
	T bs;         //为栈底元素构造的数据实体（通常不变）
	Elem *del();       //删除栈顶元素并返回下一个元素的地址（私有函数，仅用于实现类功能）

};

/**************构造函数*************/
template<class T>
Stack<T>::Stack(T bottom_date)
{
	Elem *p;
	p=new Elem;         //新增第一个栈底元素
	top=p;              //初始化栈顶地址
	bottom=p;           //初始化栈底地址
	length=0;           //初始化栈长


	bs=bottom_date;        //初始化栈底元素实体内容
	s=bs;               //初始化用来保存的栈顶元素的数据实体
	p->date=&bs;        //初始化栈底元素
	p->next=NULL;

}
template<class T>
Elem *Stack<T>::del()
{
	Elem *t=top->next;   //临时保存下一个元素地址
	delete top->date;    //删除数据实体
	delete top;          //删除元素结构
	return t;            //返回下一个元素地址
}
template<class T>
void Stack<T>::push(T s)
{
	T *date=new T; //此处必须用new申请一个不随函数销毁的字符串空间！！（此空间在主函数里申请会数据覆盖，直接声明会销毁）
	*date=s;                 //为字符串空间赋值
	Elem *p=new Elem;   //新增一个元素
	p->next=top;        //初始化next指针（指向当前栈顶地址）
	p->date=date;       //初始化date指针（指向接口输入的数据地址）
	length++;           //栈长加一
	top=p;              //改变栈顶地址（指向新建元素地址）



}
template<class T>
T Stack<T>::pop()
{
	if(top->next!=NULL)
	{
		s=*(T *)(top->date);     //栈顶数据实体临时保存至s
		Elem *p=del();      //删除栈顶元素并返回下一个元素地址
		top=p;              //修改栈顶地址（后移）
		length--;           //栈长减一
		return s;
	}
	else
	{
		s=*(T *)(top->date);     //此句必定是栈底元素数据实体保存至s（调试完成后可简化该语句）
		return s;
	}
}
template<class T>
void Stack<T>::clear()
{
	while((top->next)!=NULL)
	{
		top=del();                        //若下一个元素地址不为空，则删除
	}
}

#endif //STACK_H
