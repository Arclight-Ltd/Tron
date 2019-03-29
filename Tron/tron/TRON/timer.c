#include"io.h"
#include"init.h"

struct timerctls*timerctl=(struct timerctls*)0x003d2000;

struct timers*timeralloc(unsigned int timeout,char data,struct fifo32*fifo)			//给计时器分配内存空间子程序
{			//要计数的时间      //该计时器的标记//该计时器的缓冲区
	struct timers*timerx;
	timerx=allocbyte(sizeof(struct timers));			//获取空间		
	timerx->timeout=timeout;					//设置计时器各参数
	timerx->data=data;
	timerx->next=0;		
	timerx->fifo=fifo;
	timerx->status=TIMEROFF;
	return timerx;								
}
						
void timerfree(struct timers*timerx)				//计时器内存释放子程序
{	//计时器地址							//运用链表时交替使用两个结构体指针能使程序方便
	struct timers*pre=timerctl->timerhead,*c;
	for(c=timerctl->timerhead;c!=0;pre=c,c=c->next)				//检查计时器链表中有无该计时器					
		if(c==timerx)							//如果有
		{
			if(pre==c)						//如果是链表头
			{
				io_cli();
				timerctl->timerhead=timerctl->timerhead->next;	//将链表头指向原表头的下面一个计时器（若只有一个计时器则指向0）
				io_sti();
				freebyte(timerx,sizeof(struct timers));		//释放该计时器的内存
				return;
			}
			pre->next=c->next;					//如果是表身则将该计时器从链表中撤走
			freebyte(timerx,sizeof(struct timers));
			return;
		}	
}
						
void timeron(struct timers*timerx)				//开启计时器子程序（想办法再缩短屏蔽中断时进行的处理）
{	//计时器地址					
	struct timers*pre,*c;							//因为有哨兵垫底故不可能出现没有链表头或要接到链表尾的情况
	int eflag;
	if(timerx->status==TIMEROFF)					//如果该计时器原本是关着的才执行开启程序（避免链表陷入死循环）
	{
		eflag=io_popeflag();					//为什么要备份标志寄存器？
		io_cli();							//万一期间产生中断就会产生混乱故要在一开头就屏蔽中断
		pre=timerctl->timerhead;					//pre指向正在工作的计时器的链表头
		timerx->counter=timerctl->counter + timerx->timeout;		//该计时器的counter=当前管理表的counter+该计时器所计时间
		for(c=timerctl->timerhead;c!=0;pre=c,c=c->next)			//检测计时器链表		
			if(timerx->counter <= c->counter)			//如果找到一个预约时间比timerx早的
			{
				timerx->status=TIMERON;				//计时器为工作状态
				if(pre==c)					//如果是比链表头还要早
				{				
					timerx->next=timerctl->timerhead;	//原链表头接在timerx后面
					timerctl->timerhead=timerx;		//timerx成为新的链表头
					io_pusheflag(eflag);
					return;
				}
				timerx->next=c;					//如果是表身则插入pre结构体和c结构体的中间
				pre->next=timerx;
				io_pusheflag(eflag);				//恢复屏蔽中断前的标志寄存器值，相当于打开中断
				return;
			}
	}
	
}

void timersolder(void)						//创建哨兵子程序
{
	struct timers*solder;
	solder=timeralloc(0xffffffff,0xff,0);			//创建一个哨兵一直存在计时器链表中（压箱底）（不设置缓冲区）
	solder->counter=0xffffffff;
	timerctl->timerhead=solder;				//初始化链表头指向哨兵（若在初始化内存管理表前创建哨兵，哨兵会被清空）
	solder->status=TIMERON;					//打开哨兵计时器
}
