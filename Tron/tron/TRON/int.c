#include<stdio.h>
#include"io.h"
#include"init.h"

extern struct fifo32 mainfifo;
extern struct fifo32 mousefifo;
extern struct timerctls*timerctl;
extern struct timers*multimer;
extern struct taskctls*taskctl;


/****************************************************异常产生的中断**********************************************************************/

int*int_0dh(int *esp)							//一般保护异常中断被调用子程序
{
	char*s=0;
	struct tasks *task = taskctl->taskhead;
	putstr0("		General Protected Exception.\n");		//显示提示符
	sprintf(s,"			EIP=%08X\n",esp[11]);		//esp的值由intfunc.nas压入栈中。一个形参只能从栈中取出一个数据。此处会将[esp+4]的数据取出并当作地址赋给esp，以下的操作只能对该地址进行而无法再取得[esp+4]以后的数据
	putstr0(s);							//调用时栈中数据的分布可由intfunc中的调用程序推出或查阅P451
	return &(task->tss.esp0);						//强制结束应用程序并返回到操作系统
}

int*int_0ch(int *esp)							//栈异常中断被调用子程序
{
	char*s=0;
	struct tasks *task = taskctl->taskhead;
	putstr0("		Stack Exception.\n");		//显示提示符
	sprintf(s,"		  EIP=%08X\n",esp[11]);			//显示出错指令的esp（相对于应用程序.hrb的esp）
	putstr0(s);
	return &(task->tss.esp0);						//强制结束应用程序并返回到操作系统
}

/*****************************************************IRQ中断****************************************************************************/

void int_20h(int *esp)						//0号定时器中断被调用子程序
{
	char ts=0;
	io_out8(PIC0_OCW2, 0x60);						//通知主PIC已经接收到中断通知，使PIC继续监视IRQ0的中断情况
	timerctl->counter++;								//总counter加一
check:	if(timerctl->counter==timerctl->timerhead->counter)		//只比较链表头计时器的预约时间是否到达 以节省中断运行时间
	{	
		if(timerctl->timerhead==multimer)
			ts=1;
		else
			writebuf(timerctl->timerhead->fifo,timerctl->timerhead->data);		//将该计时器的标记送到该计时器的buf中
		timerctl->timerhead->status=TIMEROFF;			//关闭该计时器
		timerctl->timerhead=timerctl->timerhead->next;		//工作计时器链表头指向下一个预约的计时器
		goto check;						//重新检查新的表头是否到达预约时间以防止错过预约时间与上一个相同的计时器
	}
	if(ts)
		switask();
}

void int_21h(int*esp)						//1号键盘中断被调用子程序
{
	io_out8(PIC0_OCW2, 0x61);					//通知主PIC已经接收到中断通知，使PIC继续监视IRQ1的中断情况
	writebuf(&mainfifo,io_in8(KEYBO_DATA)+KEYBOFIFO);		//从键盘端口获取按下键的ascll码并储存在buf中
}

void int_27h(int *esp)						//7号中断被调用子程序
{									
	io_out8(PIC0_OCW2, 0x67);					//通知PIC已经接收到中断通知，使PIC继续监视IRQ7的中断情况
									/* PIC0からの不完全割り込み対策 
									 Athlon64X2機などではチップセットの都合によりPICの初期化時にこの割り込みが1度だけおこる 
									 この割り込み処理関数は、その割り込みに対して何もしないでやり過ごす 
									 なぜ何もしなくていいの？
									この割り込みはPIC初期化時の電気的なノイズによって発生したものなので、
									まじめに何か処理してやる必要がない。		*/						
}

void int_2ch(int*esp)						//12号鼠标中断被调用子程序
{
	io_out8(PIC1_OCW2, 0x64);					//通知从PIC已经接收到中断通知，使PIC继续监视IRQ12（在从PIC中号码为4）的中断情况
	io_out8(PIC0_OCW2, 0x62);					//通知主PIC已经接收到中断通知，使PIC继续监视IRQ2（从PIC）的中断情况
	writebuf(&mousefifo,io_in8(KEYBO_DATA));			//从键盘端口获取鼠标传来的数据并储存在buf中
}

/**********************************************************API***************************************************************************/


int *int_40h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{								//字符串显示API
	int i;
	int *reg;
	int filestackbase = *((int *) 0xfe8);				//应用程序在内存临时首地址
	struct tasks *task =taskctl->taskhead;	
	switch(edx)
	{
	case 1:putchar(eax & 0xff);break;				//一号功能：显示单个字符（eax=要显示字符）
	case 2:putstr0((char *)ebx+filestackbase);break;		//二号功能：显示字符串，以‘\0’结束（ebx=字符串首地址）
	case 3:putstr1((char *)ebx+filestackbase,ecx);break;		//三号功能：显示指定长度的字符串（ebx=字符串首地址，ecx=字符个数）
	case 4:	return &(task->tss.esp0);				//四号功能：从应用程序返回到操作系统
	case 5:	reg=&eax+1;						//五号功能：获取缓冲区数据
		while(!bufstatus(&task->appsheet.fifo));		//如果没有数据就一直等到有数据	
		io_cli();
		i=readbuf(&task->appsheet.fifo);			//从与该任务绑定的app缓冲区中读取数据					
		io_sti();			
		reg[7]=i;			
	}
	return 0;
}

int *int_41h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{								//窗口显示API
	int filestackbase = *((int *) 0xfe8);				//应用程序在内存临时首地址
	int *reg;
	struct sheets*appwin;						//应用程序的窗口
	struct tasks *task =taskctl->taskhead;	
	switch(edx)
	{
	case 1:	reg=&eax+1;						//一号功能：创建应用程序窗口 eax地址的后一位相当于esp指向eax的后一位
		appwin=shealloc((char*)ebx+filestackbase,esi,edi,eax);		//凡涉及应用程序的数据地址都要+filestackbase
		task->appsheet.sheet[task->appsheet.num]=appwin;		//登记在总图层表中的同时也登记在app专用的图层表
		task->appsheet.num++;						//登记在应用程序用的图层管理表上
		appwin->fifo=&task->appsheet.fifo;				//用task中已经申请好的缓冲区赋值给app用的缓冲区
		displaywindow(appwin,(char*)ecx+filestackbase,1);		//ebx=缓冲区，esi=xsize，edi=ysize，eax=透明色，ecx=窗口名称
		slide(appwin,200,100);
		top(appwin);
		reg[7]=(int)appwin;						//reg[0]=edi………………reg[4]=ebx………………reg[7]=eax   将图层结构体地址作为返回值
		break;
	case 2:	appwin=(struct sheets*)ebx;				//二号功能：在窗口上显示字符串
		displaywords((char*)(ebp+filestackbase),esi,edi,ecx,appwin,eax,0);break;
	case 3:	appwin=(struct sheets*)ebx;				//三号功能：在窗口上显示矩形
		squer(eax,esi,ecx,edi,appwin->buf,appwin->xsize,ebp);
		refresh(appwin->x+eax,appwin->x+esi,appwin->y+ecx,appwin->y+edi);break;
	case 4:	shefree((struct sheets*)ebx);break;			//四号功能：关闭窗口（应用窗口释放程序集成到了shefree函数中）
	}
	return 0;
}

int *int_42h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{								//内存管理API
	int filestackbase = *((int *) 0xfe8);
	struct manage*mem=(struct manage*)(ebx+filestackbase);		//ebx=内存管理表在应用文件的相对首地址
	unsigned int c,d;						//内存区块标记
	void *addr;							//暂存首地址	
	int*reg=&eax+1;	
	switch(edx)
	{
	case 1:	mem->block=1;					//功能一：初始化应用程序内存管理表（共32KB）
		mem->freememo=ecx;					//ecx=内存大小
		mem->lost=0;	
		mem->lostsize=0;	
		mem->memo[0].addr=(void*)eax;				//由于eax是给应用程序使用的所以不需要+filestackbase
		mem->memo[0].size=ecx;break;					//eax=可用内存空间的起始地址
	case 2:	for(c=0;c<mem->block;c++)			//功能二：分配内存空间	
			if(ecx<=mem->memo[c].size)			//如果有  （ecx=请求分配的字节数）
			{
				addr=mem->memo[c].addr;			//将该内存块首地址交给addr
				mem->memo[c].addr+=ecx;			//该内存块首地址后移
				mem->memo[c].size-=ecx;			//内存块的可用内存大小减少
				mem->freememo-=ecx;			//总可用内存减少
				if(!mem->memo[c].size)			//如果该内存块大小为0
				{
					mem->block--;			//去掉该内存块
					for(;c<mem->block;c++)		//该内存块之后的内存块前移覆盖
						mem->memo[c]=mem->memo[c+1];				
				}
				reg[7]=(int)addr;break;			//返回首地址（eax=分配到的内存地址）
			}
	case 3:	for(c=0;c<mem->block;c++)			//功能三：释放内存空间
			if(eax<(int)mem->memo[c].addr)			//eax=需要释放的内存空间首地址
				break;
		if(c>0)								//保证[c-1]不越界
			if(mem->memo[c-1].addr+mem->memo[c-1].size==(int*)eax)  	//当要释放的首地址与前面一个内存块能连在一起时
			{
				mem->memo[c-1].size+=ecx;				//前一内存块可用内存大小增加（ecx=释放空间的大小）
				mem->freememo+=ecx;					//总可用内存增加
				if((int*)eax+ecx==mem->memo[c].addr)			//如果要释放的首地址又与后一内存块能连在一起
				{
					mem->memo[c-1].size+=mem->memo[c].size;	//将后一内存块的大小增加到前一内存块中
					mem->block--;				//内存块数量减少
					for(;c<mem->block;c++)			//内存块前移覆盖
						mem->memo[c]=mem->memo[c+1];
				}break;					
			}
		if((int*)eax+ecx==mem->memo[c].addr)			//当要释放的首地址不能与前一内存块连在一起但能和后一个内存块能连在一起时
		{
			mem->memo[c].addr=(int*)eax;			//后一内存块首地址变为要释放内存的首地址
			mem->memo[c].size+=ecx;				//后一内存块可用内存大小增加
			mem->freememo+=ecx;				//总可用内存增加
			break;
		}			
		if(mem->block<BLOCKNUM)					//如果前后都不能连并且内存块数量在数组大小范围内
		{
			for(d=mem->block;d>c;d--)			//从后一内存块（含）开始全部内存块后移
				mem->memo[d]=mem->memo[d-1];
			mem->memo[c].addr=(int*)eax;			//将要释放的内存的信息写于“后一”内存块原本的位置
			mem->memo[c].size=ecx;
			mem->freememo+=ecx;				//总可用内存增加
			mem->block++;					//内存块数量增加
			break;
		}
		else							//如果内存块数已满
		{
			mem->lostsize+=ecx;				//将当前要释放的内存舍弃并记录入舍弃的内存大小
			mem->lost++;					//记录舍弃内存的次数
			break;
		}	
	}
	return 0;
}

int *int_43h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct tasks *task =taskctl->taskhead;
	struct timers*timer;
	int*reg=&eax+1;
	switch(edx)
	{
	case 1:	timer=timeralloc(ebx,ecx,&task->appsheet.fifo);
		task->apptimer.timer[task->apptimer.num]=timer;
		task->apptimer.num++;
		reg[7]=(int)timer;break;
	case 2:	timer=(struct timers*)ebx;
		timeron(timer);break;
	case 3:	timer=(struct timers*)ebx;
		timerfree(timer);
		task->apptimer.num--;
	}
	return 0;
}
