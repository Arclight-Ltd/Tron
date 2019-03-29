#include"init.h"
#include"io.h"
#include<stdio.h>
#include<string.h>
#define CMDNUM	11				/*��������ʱ�������޸�CMDNUM��ֵ*/

extern struct taskctls*taskctl;
extern struct shectls*shectl;	
extern struct fifo32 mainfifo;	
extern struct sheets*winnow;		
	
static char*order[CMDNUM]=										
{
	0,
	"NAME","MEM","CLS","FILE","RUN","CMD","EXIT","WHAT ARE YOU","FATHER"		/*ѯ��ϵͳ���֣���ѯ�ڴ���Ϣ����������ѯ�������ļ���Ϣ*/
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
	struct tasks*console;					//�����ݼ����ϵ��consoletask--conc.sheet->fifo=consoletask-----|
	unsigned char*windowbuf_c;				//		    	|------appsheet.fifo.task=consoletask---|
	struct sheets*window_c;
	console=taskalloc(&consolemain);				//��������������
	console->priority=3;						//�趨����������������ִ��ʱ��Ϊ0.03��z
	
	windowbuf_c=alloc(256*2*165*2);					//��ȡ��������
	window_c=shealloc(windowbuf_c,256*2,165*2,-1);			//��ȡ����ͼ��
	displaywindow(window_c,"Console",1);				//�򴰿�ͼ��д���ʼ��ͼ��
	squer(4,256*2-3,24,165*2-5,windowbuf_c,256*2,0);		//�ı������
	slide(window_c,1024/2-256,768/2-165);				//���ô��ڳ�ʼ������
	
	console->conc.sheet=window_c;					//��������ͼ���ַ������conc��
	
	window_c->fifo=allocbyte(sizeof(struct fifo32));		//��ȡ������Ļ�����//���û��������ͼ���
	initbuf(window_c->fifo);					//��ʼ�������������õĻ�����
	window_c->fifo->task=console;					//����������û�������			
	
	initbuf(&(console->appsheet.fifo));				//��ʼ����������app�õĻ�������һ�������о���һ��app�û�������
	console->appsheet.fifo.task=console;				//��app�û�������task�趨Ϊ��Ӧ������������
	
	taskwake(console);						//��������������
}

void consolemain(void)					//�����е�������							
{
	unsigned char cursor=0,cmdws[60],cmdw=0;			//�ǣ�1����0�������ѡ�У�����ָ���õ��ַ����飬��д���ַ�������
	unsigned char cursor_c=7;					//���Ĭ��Ϊ��ɫ
	unsigned int i;						
	struct timers*timer1;
	struct tasks*contask;
	struct fifo32*confifo;						//ע���ʱ��timefifo��ָ�롣��ֵΪ�������ĵ�ַ
	
	contask=taskctl->taskhead;					//��ȡ������ĵ�ַ
	
	contask->conc.cursor_x=1;					//�ַ��봰������˵ľ���Ϊ1���ַ�
	contask->conc.cursor_y=0;					//�ַ��봰�����϶˵ľ���Ϊ28�����أ���0�У�
	confifo=contask->conc.sheet->fifo;				//��ȡ�����л�����
	
	top(contask->conc.sheet);					//��������������ʱ���������д����ö�
		
	putchar('>');							//��ʾ������ʾ��
	contask->conc.mark_y=contask->conc.cursor_y;			//���浱ǰ��ʾ��������
	
	timer1=timeralloc(50,0,confifo);				//���ü�ʱ��1
	timeron(timer1);
	
	while(1)										
	{		
		io_cli();						//��ȡ����ǰ�����ж�
		if(bufstatus(confifo))					//��������񻺳����������ݾ�ִ��
		{	
			i=readbuf(confifo);				//��ȡ����					
			io_sti();					//�����ж�
			switch(i)
			{
			case -1:cursor_c=7;				//������ʾ��ɫ�Ի�øյ���ͻ�÷�Ӧ��Ч��
				cursor=1;				//˵�������������뽹�㣬��ʼ��ʾ���
				timer1->data=0;				//Ԥ����һ�����Ϊ��ɫ������ʾ���ΰ�ɫ
				timeron(timer1);			//�򿪶�ʱ��
				break;					
			case -2:drawcursor(0);				//˵��������ʧȥ���뽹�㣬ȡ�������˸������Ϳ�ڣ�
				cursor=0;		
				break;
			case 8:			//������˸��
				if(!((contask->conc.cursor_x<=2)&&(contask->conc.cursor_y==contask->conc.mark_y)))	//ֻ����һֱ�˸������һ��>Ϊֹ
				{
					drawcursor(0);			//������
					contask->conc.cursor_x--;
					if(contask->conc.cursor_x==0)			//�����굽�˸������׾��л�����һ�е�ĩβ
					{
						contask->conc.cursor_x=62;
						contask->conc.cursor_y--;
					}
					drawcursor(7);			//�ָ���ʾ���
					if(cmdw>0)
						cmdw--;					//�����ַ�ָ���һ
				}break;	
			case 10:		//����ǻس���
				drawcursor(0);				//������
				newline();					//����
				if(cmdw)					//����ַ�ָ��>0˵���ַ������ַ�
					cmd(cmdws,&cmdw);			//�ͷ����ַ���	
				putchar('>');					//��ʾ������ʾ��
				contask->conc.mark_y=contask->conc.cursor_y;			//��¼��ʱ>���ŵ��к�
				break;
			case KILL:	timerfree(timer1);			//����֪ͨ������Ҫ���ͷ�ʱ�����ͷż�ʱ���ռ�
					writebuf(&mainfifo,KILL);		//��task_a�������������ͷ�
					tasksleep(contask);			//���������task_a���ȼ��ȸ������ʱ���ø��������ߣ�һ����䲻�ᱻִ�У�
			default:		//����Ǽ�������
				if(i>=KEYBOFIFO)				//������ַ�����
				{
					i-=KEYBOFIFO;				//�����ݻָ�Ϊԭ������ֵ
					putchar((char)i);			//��ʾ���ַ�
					cmdws[cmdw]=(char)i;			//�����ַ����浽�����ַ�������
					cmdw++;
					if(cmdw>59)				//��֤�����ַ������鲻Խ��
						cmdw=59;
				}		
			}				
			if(cursor==1)
				switch(i)		//����Ƕ�ʱ������		//�����ʾ
				{
				case 0:cursor_c=0;timer1->data=1;timeron(timer1);break;
				case 1:cursor_c=7;timer1->data=0;timeron(timer1);
				}
		}
		else				//���û����
		{
			io_sti();
			tasksleep(contask);					//��������	
			io_sti();
		}
		if(cursor==1)							//��������ѡ��
			drawcursor(cursor_c);					//���ƹ��
	}			
}

void drawcursor(unsigned char cursor_c)					//���ƹ���ӳ���
{		//��ǰ���λ�ýṹ��//�����ɫ
	struct conchar*conc=&winnow->fifo->task->conc;			//conc��task��
	squer(conc->cursor_x*8,conc->cursor_x*8+8,28+conc->cursor_y*16,28+16+conc->cursor_y*16,conc->sheet->buf,conc->sheet->xsize,cursor_c);	//�����ʾ
	refresh(conc->cursor_x*8+conc->sheet->x,conc->cursor_x*8+8+conc->sheet->x,28+conc->sheet->y+conc->cursor_y*16,28+16+conc->sheet->y+conc->cursor_y*16);//ע��ˢ�µ��Ǿ���λ��
}

void putchar(char word)							//�������д�����ʾ�����ַ��ӳ���
{		//��ǰ���λ�ýṹ��//Ҫ��ʾ���ַ�
	char s[2];
	struct conchar*conc=&winnow->fifo->task->conc;			//����ѡ�д��ڰ󶨵�task����ֵ������taskhead��ʹ��������task_aʱҲ��ʹ��putchar�Ⱥ���
	s[0]=word;								//��Ҫʱֱ�Ӵ��ڴ���ȡ������		
	s[1]=0;									//ע�⣺���ݱ�ʾ���ڴ������ֻ����ת��Ϊ��int*����
	switch(word)
	{
	case 0x09:	conc->cursor_x+=8;					//������Ʊ��\t
			if(conc->cursor_x>62)							//�����굽�˸��е�ĩβ
			{
				conc->cursor_x=1;						//��굽��һ������
				newline();	
			}break;
	case 0x0a:	newline();break;					//����ǻ��з�\n
	case 0x0d:	conc->cursor_x=1;break;					//����ǻس���\r
	default:	displaywords(s,conc->cursor_x*8,28+conc->cursor_y*16,1,conc->sheet,7,0);	//�������ͨ�ַ�
			conc->cursor_x++;
			if(conc->cursor_x>62)							//�����굽�˸��е�ĩβ
			{
				conc->cursor_x=1;						//��굽��һ������
				newline();	
			}
	}
}

void putstr0(char*s)				//�������д�����ʾ�ַ�����\0���ᣩ�ӳ���
{	//�ַ����׵�ַ
	for(;*s!=0;s++)
		putchar(*s);
}

void putstr1(char*s,int num)			//�������д�����ʾָ�������ַ��ӳ���
{	//�ַ����׵�ַ//����
	int c;
	for(c=0;c<num;c++)
		putchar(s[c]);
}

void newline()						//�����ӳ���
{
	unsigned int x,y;
	struct conchar*conc=&winnow->fifo->task->conc;
	conc->cursor_x=1;
	(conc->cursor_y)++;
	if(conc->cursor_y==18)				//�������Խ���ʹ��Ļ����
	{
		conc->cursor_y=17;				//���������������һ��
		for(x=8;x<256*2-3;x++)			//>��ʾ�������<X<�����߽�
			for(y=28;y<165*2-5-16;y++)	//>��ʾ�����ϲ�<Y<�����߽�
				conc->sheet->buf[x+y*conc->sheet->xsize]=conc->sheet->buf[x+(y+16)*conc->sheet->xsize];
		squer(4,256*2-3,165*2-5-16,165*2-5,conc->sheet->buf,conc->sheet->xsize,0);
		refresh(4+conc->sheet->x,256*2-3+conc->sheet->x,24+conc->sheet->y,165*2-5+conc->sheet->y);
	}
}

void cmd(char*cmdws,unsigned char*cmdw)					//���������ӳ���
{
	unsigned char num;
	unsigned long cn;						//������
	void(*cf)(char*);						//����ָ������ָ��
	for(num=0;num<*cmdw;num++)					//��������ַ���ȫ��ת��Ϊ��д��֧�ִ�Сд����
		if(('a'<=cmdws[num])&&(cmdws[num]<='z'))		//ֻ��Сд�ַ�ת��Ϊ��д
			cmdws[num]&=0xdf;				//���⽫�ո�(00100000)���ַ�ת��Ϊ0������strstr��������Ϊ�ַ�������
	cmdws[*cmdw]=0;							//����ַ�����β�Ա������ɾ���˵��ַ�
	cn=cmdcmp(cmdws);						//���������
	cf=cmdfunc[cn];							//����Ŷ�Ӧ�ĺ�����ڵ�ַ����cf
	cf(cmdws);							//ִ�иú���
	for(num=0;num<60;num++)						//��ȫ��մ����ַ����������				
		cmdws[num]=0;
	*cmdw=0;							//�����ڱ�������ַ���д��λ�õı�����0
}

unsigned long cmdcmp(char*cmdws)					//�Ƚ�����������Ƿ��а�������ؼ���
{
	unsigned char p;
	char*head=cmdws,*po=0;			//headָ��������ַ������׵�ַ��cmdָ�����趨�����һ���ַ�������ַ��������ַ����еĵ�ַ
	unsigned long cn;
check:	for(cn=1;cn<CMDNUM;cn++)				//���ҳ��������һ���ַ�������������ַ����еĵ�ַ
	{
		po=strstr(head,order[cn]);			//���û��po=0��strstr��head�ַ�������0��Ϊhead�ַ���������־��
		if(po)						//һ���ҵ�һ�����ܵ����
			break;
	}
	if(po)							//�����
	{
		for(p=0;*(order[cn]+p)!='\0';p++)		//���жϽ����ŵļ����ַ��Ƿ�������Ƿ���ؼ�����ȫ�����
			if(po[p]!=*(order[cn]+p))		//���������ȫ���
			{
				head=po+1;			//headָ��po�������ַ������ַ�����һ���ַ�������������ѭ����
				goto check;			//�������ң���ʱֻ��Ҫ����ʣ�µ������ַ������ɣ�
			}
		return cn;					//��������Ͼͷ��ظ�����ı�ţ���cmd�����е���ţ�
	}	
	else
		return 0;					//���û�оͷ���0
}
