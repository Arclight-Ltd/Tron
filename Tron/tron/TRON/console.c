#include"init.h"
#include"io.h"
#include<stdio.h>
#include<string.h>
#define CMDNUM	11				/*增加命令时别忘了修改CMDNUM的值*/

extern struct taskctls*taskctl;
extern struct shectls*shectl;	
extern struct fifo32 mainfifo;	
extern struct sheets*winnow;		
	
static char*order[CMDNUM]=										
{
	0,
	"NAME","MEM","CLS","FILE","RUN","CMD","EXIT","WHAT ARE YOU","FATHER"		/*询问系统名字，查询内存信息，清屏，查询磁盘中文件信息*/
	,0
};

static void(*cmdfunc[CMDNUM])(char*cmdws)=
{
	error,
	name,mem,cls,dir,run,opencmd,exit,whatareyou,father
	,error
};

void initconsole(void)
{								//		   	|---------------------------------------|
	struct tasks*console;					//各数据间的联系：consoletask--conc.sheet->fifo=consoletask-----|
	unsigned char*windowbuf_c;				//		    	|------appsheet.fifo.task=consoletask---|
	struct sheets*window_c;
	console=taskalloc(&consolemain);				//分配命令行任务
	console->priority=3;						//设定分配给命令行任务的执行时间为0.03秒z
	
	windowbuf_c=alloc(256*2*165*2);					//获取窗口数组
	window_c=shealloc(windowbuf_c,256*2,165*2,-1);			//获取窗口图层
	displaywindow(window_c,"Console",1);				//向窗口图像写入初始化图形
	squer(4,256*2-3,24,165*2-5,windowbuf_c,256*2,0);		//文本输入框
	slide(window_c,1024/2-256,768/2-165);				//设置窗口初始化坐标
	
	console->conc.sheet=window_c;					//将命令行图层地址保存在conc中
	
	window_c->fifo=allocbyte(sizeof(struct fifo32));		//获取该任务的缓冲区//将该缓冲区与该图层绑定
	initbuf(window_c->fifo);					//初始化命令行任务用的缓冲区
	window_c->fifo->task=console;					//将该任务与该缓冲区绑定			
	
	initbuf(&(console->appsheet.fifo));				//初始化该命令行app用的缓冲区（一个命令行就有一个app用缓冲区）
	console->appsheet.fifo.task=console;				//将app用缓冲区的task设定为对应的命令行任务
	
	taskwake(console);						//启动命令行任务
}

void consolemain(void)					//命令行的主程序							
{
	unsigned char cursor=0,cmdws[60],cmdw=0;			//是（1）否（0）被鼠标选中，储存指令用的字符数组，已写入字符的数量
	unsigned char cursor_c=7;					//光标默认为白色
	unsigned int i;						
	struct timers*timer1;
	struct tasks*contask;
	struct fifo32*confifo;						//注意此时的timefifo是指针。其值为缓冲区的地址
	
	contask=taskctl->taskhead;					//提取该任务的地址
	
	contask->conc.cursor_x=1;					//字符与窗口最左端的距离为1个字符
	contask->conc.cursor_y=0;					//字符与窗口最上端的距离为28个像素（第0行）
	confifo=contask->conc.sheet->fifo;				//获取命令行缓冲区
	
	top(contask->conc.sheet);					//运行任务行任务时将该命令行窗口置顶
		
	putchar('>');							//显示命令提示符
	contask->conc.mark_y=contask->conc.cursor_y;			//储存当前提示符所在行
	
	timer1=timeralloc(50,0,confifo);				//设置计时器1
	timeron(timer1);
	
	while(1)										
	{		
		io_cli();						//读取数据前屏蔽中断
		if(bufstatus(confifo))					//如果该任务缓冲区中有数据就执行
		{	
			i=readbuf(confifo);				//读取数据					
			io_sti();					//开放中断
			switch(i)
			{
			case -1:cursor_c=7;				//马上显示白色以获得刚点击就获得反应的效果
				cursor=1;				//说明该任务获得输入焦点，开始显示光标
				timer1->data=0;				//预定下一个光标为黑色以免显示两次白色
				timeron(timer1);			//打开定时器
				break;					
			case -2:drawcursor(0);				//说明该任务失去输入焦点，取消光标闪烁（马上涂黑）
				cursor=0;		
				break;
			case 8:			//如果是退格键
				if(!((contask->conc.cursor_x<=2)&&(contask->conc.cursor_y==contask->conc.mark_y)))	//只允许一直退格到最近的一个>为止
				{
					drawcursor(0);			//清除光标
					contask->conc.cursor_x--;
					if(contask->conc.cursor_x==0)			//如果光标到了该行行首就切换到上一行的末尾
					{
						contask->conc.cursor_x=62;
						contask->conc.cursor_y--;
					}
					drawcursor(7);			//恢复显示光标
					if(cmdw>0)
						cmdw--;					//输入字符指针减一
				}break;	
			case 10:		//如果是回车键
				drawcursor(0);				//清除光标
				newline();					//换行
				if(cmdw)					//如果字符指针>0说明字符串有字符
					cmd(cmdws,&cmdw);			//就分析字符串	
				putchar('>');					//显示命令提示符
				contask->conc.mark_y=contask->conc.cursor_y;			//记录此时>符号的行号
				break;
			case KILL:	timerfree(timer1);			//当被通知该任务要被释放时，先释放计时器空间
					writebuf(&mainfifo,KILL);		//向task_a发送数据请求释放
					tasksleep(contask);			//保险起见（task_a优先级比该任务低时）让该任务休眠（一般这句不会被执行）
			default:		//如果是键盘数据
				if(i>=KEYBOFIFO)				//如果是字符数据
				{
					i-=KEYBOFIFO;				//将数据恢复为原来的数值
					putchar((char)i);			//显示该字符
					cmdws[cmdw]=(char)i;			//将该字符储存到输入字符串数组
					cmdw++;
					if(cmdw>59)				//保证输入字符串数组不越界
						cmdw=59;
				}		
			}				
			if(cursor==1)
				switch(i)		//如果是定时器数据		//光标显示
				{
				case 0:cursor_c=0;timer1->data=1;timeron(timer1);break;
				case 1:cursor_c=7;timer1->data=0;timeron(timer1);
				}
		}
		else				//如果没数据
		{
			io_sti();
			tasksleep(contask);					//任务休眠	
			io_sti();
		}
		if(cursor==1)							//如果被鼠标选中
			drawcursor(cursor_c);					//绘制光标
	}			
}

void drawcursor(unsigned char cursor_c)					//绘制光标子程序
{		//当前光标位置结构体//光标颜色
	struct conchar*conc=&winnow->fifo->task->conc;			//conc与task绑定
	squer(conc->cursor_x*8,conc->cursor_x*8+8,28+conc->cursor_y*16,28+16+conc->cursor_y*16,conc->sheet->buf,conc->sheet->xsize,cursor_c);	//光标显示
	refresh(conc->cursor_x*8+conc->sheet->x,conc->cursor_x*8+8+conc->sheet->x,28+conc->sheet->y+conc->cursor_y*16,28+16+conc->sheet->y+conc->cursor_y*16);//注意刷新的是绝对位置
}

void putchar(char word)							//在命令行窗口显示单个字符子程序
{		//当前光标位置结构体//要显示的字符
	char s[2];
	struct conchar*conc=&winnow->fifo->task->conc;			//根据选中窗口绑定的task来赋值而不是taskhead，使得在运行task_a时也能使用putchar等函数
	s[0]=word;								//需要时直接从内存中取出即可		
	s[1]=0;									//注意：数据表示的内存必须且只能先转化为（int*）型
	switch(word)
	{
	case 0x09:	conc->cursor_x+=8;					//如果是制表符\t
			if(conc->cursor_x>62)							//如果光标到了该行的末尾
			{
				conc->cursor_x=1;						//光标到下一行行首
				newline();	
			}break;
	case 0x0a:	newline();break;					//如果是换行符\n
	case 0x0d:	conc->cursor_x=1;break;					//如果是回车符\r
	default:	displaywords(s,conc->cursor_x*8,28+conc->cursor_y*16,1,conc->sheet,7,0);	//如果是普通字符
			conc->cursor_x++;
			if(conc->cursor_x>62)							//如果光标到了该行的末尾
			{
				conc->cursor_x=1;						//光标到下一行行首
				newline();	
			}
	}
}

void putstr0(char*s)				//在命令行窗口显示字符串（\0作结）子程序
{	//字符串首地址
	for(;*s!=0;s++)
		putchar(*s);
}

void putstr1(char*s,int num)			//在命令行窗口显示指定个数字符子程序
{	//字符串首地址//个数
	int c;
	for(c=0;c<num;c++)
		putchar(s[c]);
}

void newline()						//换行子程序
{
	unsigned int x,y;
	struct conchar*conc=&winnow->fifo->task->conc;
	conc->cursor_x=1;
	(conc->cursor_y)++;
	if(conc->cursor_y==18)				//如果行数越界就使屏幕滚动
	{
		conc->cursor_y=17;				//输入行锁定在最后一行
		for(x=8;x<256*2-3;x++)			//>提示符的左侧<X<输入框边界
			for(y=28;y<165*2-5-16;y++)	//>提示符的上侧<Y<输入框边界
				conc->sheet->buf[x+y*conc->sheet->xsize]=conc->sheet->buf[x+(y+16)*conc->sheet->xsize];
		squer(4,256*2-3,165*2-5-16,165*2-5,conc->sheet->buf,conc->sheet->xsize,0);
		refresh(4+conc->sheet->x,256*2-3+conc->sheet->x,24+conc->sheet->y,165*2-5+conc->sheet->y);
	}
}

void cmd(char*cmdws,unsigned char*cmdw)					//分析命令子程序
{
	unsigned char num;
	unsigned long cn;						//命令编号
	void(*cf)(char*);						//定义指向函数的指针
	for(num=0;num<*cmdw;num++)					//将输入的字符串全部转化为大写以支持大小写混用
		if(('a'<=cmdws[num])&&(cmdws[num]<='z'))		//只将小写字符转换为大写
			cmdws[num]&=0xdf;				//避免将空格(00100000)等字符转化为0而导致strstr函数误以为字符串结束
	cmdws[*cmdw]=0;							//标记字符串结尾以避免误读删除了的字符
	cn=cmdcmp(cmdws);						//获得命令编号
	cf=cmdfunc[cn];							//将编号对应的函数入口地址赋给cf
	cf(cmdws);							//执行该函数
	for(num=0;num<60;num++)						//完全清空储存字符输入的数组				
		cmdws[num]=0;
	*cmdw=0;							//将用于标记输入字符串写入位置的变量清0
}

unsigned long cmdcmp(char*cmdws)					//比较输入的命令是否有包含命令关键字
{
	unsigned char p;
	char*head=cmdws,*po=0;			//head指向输入的字符串的首地址，cmd指向与设定命令第一个字符相符的字符在输入字符串中的地址
	unsigned long cn;
check:	for(cn=1;cn<CMDNUM;cn++)				//先找出与命令第一个字符相符的在输入字符串中的地址
	{
		po=strstr(head,order[cn]);			//如果没有po=0（strstr以head字符串出现0作为head字符串结束标志）
		if(po)						//一旦找到一个可能的情况
			break;
	}
	if(po)							//如果有
	{
		for(p=0;*(order[cn]+p)!='\0';p++)		//再判断紧接着的几个字符是否相符（是否与关键词完全相符）
			if(po[p]!=*(order[cn]+p))		//如果不是完全相符
			{
				head=po+1;			//head指向po在输入字符串中字符的下一个字符（避免陷入死循环）
				goto check;			//继续查找（此时只需要查找剩下的输入字符串即可）
			}
		return cn;					//如果都符合就返回该命令的编号（在cmd数组中的序号）
	}	
	else
		return 0;					//如果没有就返回0
}
