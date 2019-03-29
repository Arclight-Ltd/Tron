#include<stdio.h>
#include"io.h"
#include"init.h"									//c�������Զ���ͷ�ļ������ݸ�������λ��

extern struct mouse variable;								//������ڼ�¼������ݵĸ�������
extern struct shectls*shectl;								//ͼ������
extern struct timerctls*timerctl;							//��ʱ�������
extern struct taskctls*taskctl;
extern struct fifo32 mainfifo;
extern struct fifo32 mousefifo;
extern struct sheets*winnow;

void HariMain(void)
{
	unsigned int i;							//data��Ϊ�����ݴ�����������Ϊ�ֲ�������dealmouse�ӳ����ж���
	int x, y, r, g, b;
	char*t=0;
	struct tasks*now,*killtask;
	unsigned char*colbuf;
	struct sheets*colsheet;
	struct timers*timer1;
	char s=0,m=0,h=0;
	init();									//���ֳ�ʼ������
	now=taskctl->taskhead;
	mousefifo.task=now;
	mainfifo.task=now;
		
	colbuf=alloc(144*164);						//��ȡ��������
	colsheet=shealloc(colbuf,144,164,-1);				//��ȡ����ͼ�㣨2�ţ�
	displaywindow(colsheet,"Color",0);				//�򴰿�ͼ��д���ʼ��ͼ��
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
		io_cli();							//�����ж�					
		if(bufstatus(&mousefifo))				//���ר�û�����
		{
			i=readbuf(&mousefifo);								
			io_sti();
			dealmouse(variable.data,i);			//����ж϶�ȡ����
		}
		else
			if(bufstatus(&mainfifo))
			{
				i=readbuf(&mainfifo);								
				io_sti();
				if(i>=KEYBOFIFO)
					dealkeybored(i-KEYBOFIFO);			//�����ж϶�ȡ����
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
				case KILL:					//�ͷź��л������������񣩴���
						killtask=winnow->fifo->task;		//�ݴ�Ҫ�ͷŵ������ַ
						if(killtask)				//�����Ҫ�ͷŵ�����ʹֻ�ͷŻ������Ĵ���Ҳ��ʹ��kill��
						{
							io_cli();			//�������жϱ�֤�����������л�
							tasksleep(killtask);		//�ñ��ͷŵ��������ߣ�����Ҫ�ͷ���������ֻ��ͨ��task_a���ͷţ������Լ��ͷ��Լ�����
							winnow->fifo->task=0;		//����ô��ڻ������󶨵��������ⷢ�����ѣ���һ�û������ֻ������Ҳ�ܱ�֤���ỽ�Ѹ�����
							free((void*)killtask->tss.esp1,64*1024);		//�ͷ������ջ�ռ�
							freebyte(killtask,sizeof(struct tasks));	//�ͷŸ�������Ϣ�ṹ����ռ�ڴ�
							io_sti();
						}
						freebyte(winnow->fifo,sizeof(struct fifo32));	//�ͷŻ�����
						free(winnow->buf,winnow->xsize*winnow->ysize);	//�ͷŸ�ͼ���buf�ռ�//ֻ��bufͼ�񻺳�������ʱ����4k�ڴ���䷨ 
						shefree(winnow);				//�ͷŸ�ͼ����ռ�ռ�
						break;
				}
			}
			else							//���û�����ݾ��õ�ǰ��������
			{
				tasksleep(now);			
				io_sti();					//���߽�������ʱ������ж�
			}						
		sprintf(t,"%02d:%02d:%02d",h,m,s);					//�ڴ�������ʾ������
		displaywords(t,45,30,10,shectl->sheet[2],0,BACKCOL);							
	}									
}

void init(void)									//���ֳ�ʼ���ӳ���
{
	initbuf(&mainfifo);
	initbuf(&mousefifo);
	initint();								//��ʼ���жϺͶκ�����			
	initkeybored();								//���������Ƶ�·
	enablemouse();								//�������(������һ������жϣ���Ҫ�ȳ�ʼ��������)	
	variable.mark=0;							//���������ʼ����¼��Ϊ0�Ա������ü��������
	
	initpalette(0,15);							//���õ�ɫ��ɫ��
	initmemory();								//��ʼ���ڴ�����
	initsheet();								//��ʼ��ͼ��	
	timersolder();								//�����ڱ�	
	initmultask();								//��ʼ�������л��ṹ��,���������л����������ڱ�֮��
	initconsole();								//��ʼ������������
	readfat();								//����FAT��Ϊ��ȡӦ�ó�����׼��(ֻ��Ҫ��һ��)
}

