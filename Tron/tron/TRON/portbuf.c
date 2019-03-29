#include"init.h"

struct fifo32 mainfifo;
struct fifo32 mousefifo;

void initbuf(struct fifo32*fifo)			//初始化缓冲区子程序（接受结构体地址而不是创建临时结构体变量）
{	//缓冲区结构体地址//缓冲区数据储存数组地址//缓冲区大小（数组大小）
	fifo->size=128;								//初始化缓冲区大小
	fifo->free=128;								//初始化缓冲区剩余大小
	fifo->write=0;								//初始化缓冲区写入位置
	fifo->read=0;								//初始化缓冲区读取位置
	fifo->task=0;								//初始化指定任务为空
}

int writebuf(struct fifo32*fifo,int data)			//向缓冲区写入数据子程序
{
	if(fifo->free==0)							//如果缓冲区里已满则返回-1
		return -1;
	fifo->buf[fifo->write]=data;						//将数据写入缓冲区
	fifo->write++;								//写入位置下移一位(类比栈顶指针)
	if(fifo->write==fifo->size)						//当写入位置触及缓冲区边界时回到起点
		fifo->write=0;
	fifo->free--;								//计算缓冲区剩余大小
	if(fifo->task)								//如果有指定唤醒的任务
		if(!fifo->task->status)						//如果该任务处于休眠状态
			taskwake(fifo->task);					//唤醒该任务
	return 0;
}

int readbuf(struct fifo32*fifo)					//从缓冲区读出数据子程序
{
	int data;
	if(fifo->free==fifo->size)						//如果缓冲区里没有数据则返回-1
		return -1;
	data=fifo->buf[fifo->read];						//将数据从缓冲区读出，存入data
	fifo->read++;								//读出位置下移一位（与写入位置独立工作）
	if(fifo->read==fifo->size)						//当读出位置触及缓冲区边界时回到起点
		fifo->read=0;
	fifo->free++;								//计算缓冲区剩余大小
	return data;								//返回读出的数据
}

int bufstatus(struct fifo32*fifo)					//报告缓冲区有多少数据子程序
{
	return fifo->size-fifo->free;
}

