#include<stdio.h>
#include"API.h"

struct cursors
{
	int x,y;
};

void autopchar(int win,struct cursors*cur,char c);		//�Զ�����λ�õ���ʾ�����ַ�����

void HariMain(void)					//Ӧ�ó�����ϵͳ�������룬c����ʼִ�е����Ϊharimain����
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
			endapp();break;					//����apiʵ��Ӧ�ó��������ϵͳ����ת
		default:				//����Ǽ�������
			if(i>=256)					//������ַ�����
			{
				c=i-256;				//�����ݻָ�Ϊԭ������ֵ
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
	putstrwin(win,cur->x,cur->y,7,1,ch);				//��ʾ���ַ�
	cur->x+=8;
	if(cur->x>=384)
	{
		cur->x=8;
		cur->y+=16;
	}	
}
