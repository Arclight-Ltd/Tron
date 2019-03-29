#include<stdio.h>
#include"API.h"

struct cursors
{
	int x,y;
};

void autopchar(int win,struct cursors*cur,char c);		//自动调整位置的显示单个字符程序

void HariMain(void)					//应用程序与系统独立编译，c程序开始执行的入口为harimain程序
{	
	char*winbuf,c;
	int win,i,timer;
	struct cursors cur;
	initmalloc();
	winbuf=malloc(400*400);
	win=window(winbuf,400,400,-1,"APP1");
	squerwin(4,400-3,24,400-5,win,0);
	cur.x=8;
	cur.y=28;
	timer=timeralloc(100,1);
	timeron(timer);
	while(1)
	{	
		i=getfifo();
		switch(i)
		{	
		case 1: autopchar(win,&cur,'A');timeron(timer);break;
		case 10 :closewin(win);
			endapp();break;					//调用api实现应用程序向操作系统的跳转
		default:				//如果是键盘数据
			if(i>=256)					//如果是字符数据
			{
				c=i-256;				//将数据恢复为原来的数值
				autopchar(win,&cur,c);
			}	
		}	
	}
	
}

void autopchar(int win,struct cursors*cur,char c)
{
	char ch[2];
	ch[0]=c;
	ch[1]=0;
	putstrwin(win,cur->x,cur->y,7,1,ch);				//显示该字符
	cur->x+=8;
	if(cur->x>=384)
	{
		cur->x=8;
		cur->y+=16;
	}	
}
