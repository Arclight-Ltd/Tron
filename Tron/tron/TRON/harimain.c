#include<stdio.h>
#include"io.h"
#include"init.h"									//c编译器自动将头文件的内容复制至此位置

extern struct mouse variable;								//打包用于记录鼠标数据的各个变量
extern struct shectls*shectl;								//图层管理表
extern struct timerctls*timerctl;							//计时器管理表
extern struct taskctls*taskctl;
extern struct fifo32 mainfifo;
extern struct fifo32 mousefifo;
extern struct sheets*winnow;

void HariMain(void)
{
	unsigned int i;							//data作为数据暂存器，不能作为局部变量在dealmouse子程序中定义
	int x, y, r, g, b;
	char*t=0;
	struct tasks*now,*killtask;
	unsigned char*colbuf;
	struct sheets*colsheet;
	struct timers*timer1;
	char s=0,m=0,h=0;
	init();									//各种初始化处理、
	now=taskctl->taskhead;
	mousefifo.task=now;
	mainfifo.task=now;
		
	colbuf=alloc(144*164);						//获取窗口数组
	colsheet=shealloc(colbuf,144,164,-1);				//获取窗口图层（2号）
	displaywindow(colsheet,"Color",0);				//向窗口图像写入初始化图形
	slide(colsheet,380,72);
	for (y = 0; y < 128; y++) 
		for (x = 0; x < 128; x++) 
		{
			r = x * 2;
			g = y * 2;
			b = 0;
			colbuf[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
		}
	top(colsheet);
						
	timer1=timeralloc(100,0,&mainfifo);
	timeron(timer1);
	while(1)										
	{						
		io_cli();							//屏蔽中断					
		if(bufstatus(&mousefifo))				//鼠标专用缓冲区
		{
			i=readbuf(&mousefifo);								
			io_sti();
			dealmouse(variable.data,i);			//鼠标中断读取程序
		}
		else
			if(bufstatus(&mainfifo))
			{
				i=readbuf(&mainfifo);								
				io_sti();
				if(i>=KEYBOFIFO)
					dealkeybored(i-KEYBOFIFO);			//键盘中断读取程序
				switch(i)
				{
				case 0:	timeron(timer1);
					s++;
					if(s==60)
					{
						m++;s=0;
						if(m==60)
						{
							h++;m=0;
						}
							
					}
					break;
				case KILL:					//释放含有缓冲区（和任务）窗口
						killtask=winnow->fifo->task;		//暂存要释放的任务地址
						if(killtask)				//如果有要释放的任务（使只释放缓冲区的窗口也可使用kill）
						{
							io_cli();			//先屏蔽中断保证不发生任务切换
							tasksleep(killtask);		//让被释放的任务休眠（由于要释放任务所以只能通过task_a来释放（不能自己释放自己））
							winnow->fifo->task=0;		//清除该窗口缓冲区绑定的任务以免发生误唤醒（万一该缓冲区又获得数据也能保证不会唤醒该任务）
							free((void*)killtask->tss.esp1,64*1024);		//释放任务的栈空间
							freebyte(killtask,sizeof(struct tasks));	//释放该任务信息结构体所占内存
							io_sti();
						}
						freebyte(winnow->fifo,sizeof(struct fifo32));	//释放缓冲区
						free(winnow->buf,winnow->xsize*winnow->ysize);	//释放该图层的buf空间//只有buf图像缓冲区建立时用了4k内存分配法 
						shefree(winnow);				//释放该图层所占空间
						break;
				}
			}
			else							//如果没有数据就让当前任务休眠
			{
				tasksleep(now);			
				io_sti();					//休眠结束回来时立马打开中断
			}						
		sprintf(t,"%02d:%02d:%02d",h,m,s);					//在窗口中显示计数器
		displaywords(t,45,30,10,shectl->sheet[2],0,BACKCOL);							
	}									
}

void init(void)									//各种初始化子程序
{
	initbuf(&mainfifo);
	initbuf(&mousefifo);
	initint();								//初始化中断和段号设置			
	initkeybored();								//激活鼠标控制电路
	enablemouse();								//激活鼠标(会引发一次鼠标中断，故要先初始化缓冲区)	
	variable.mark=0;							//激活鼠标后初始化记录器为0以便舍弃该激活反馈数据
	
	initpalette(0,15);							//设置调色板色号
	initmemory();								//初始化内存管理表
	initsheet();								//初始化图层	
	timersolder();								//创建哨兵	
	initmultask();								//初始化任务切换结构体,开启任务切换（必须在哨兵之后）
	initconsole();								//初始化命令行任务
	readfat();								//解码FAT，为读取应用程序做准备(只需要做一次)
}

