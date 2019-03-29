#include"io.h"
#include"init.h"
#include<stdio.h>
					//����������ʱ��taskheadΪtask_a
extern struct shectls*shectl;
extern struct mesg*info;
extern struct fifo8 keyfifo;								//����keyfifo�ṹ��Ϊ���̻���������
extern struct fifo8 mousefifo;								//����mousefifo�ṹ��Ϊ��껺��������
extern unsigned char keybuf[32];							//����keybuf������Ϊ���̻���������ռ�
extern unsigned char mousebuf[128];							//����mousebuf������Ϊ��껺��������ռ�

char cursor_x=0,shift=0x0f;								//���λ�ã�shift��״̬
unsigned char modol=0;									//��¼�Ƿ����϶�����ģʽ
struct mouse variable;									//������ڼ�¼������ݵĸ�������
struct sheets*winnow;									//���浱ǰѡ�е�ͼ��

static char keytable_n[0x54]=								/*shiftû���µ��ַ���*/
{
	0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\\',   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'' , 0,   0, '|','Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0,   0, 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};
static char keytable_s[0x54] = 								/*shift���µ��ַ���*/
{
	0,   0,   '!', '@', '#', '$', '%', '^', '&', '*','(', ')', '_', '+', 0,   0,  
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|',   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0,   0,   '}', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0,   0, 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};


void waitkbc(void)					//�ȴ����̿��Ƶ�·��Ӧ�ӳ���
{
	while(io_in8(KEYBO_STATUS)&0x02);		//�Ӽ���״̬KEYBO_STATUS�˿ڴ���ȡ�����ݵĵ�λ�����ڶ�λΪ0��˵����·��Ӧ�����Խ���CPU����
}

void initkeybored(void)					//��ʼ����������̿��Ƶ�·�ӳ��������Ƶ�·���ڼ��̿��Ƶ�·�У�
{
	waitkbc();
	io_out8(KEYBO_COMMAND,0x60);			//���������KEYBO_COMMAND�˿����ģʽ�趨ָ��
	waitkbc();
	io_out8(KEYBO_DATA,0x47);			//���������KEYBO_DATA�˿����ģʽ���롪��0x47�����ģʽ��
}

void enablemouse(void)					//��������ӳ���
{
	waitkbc();
	io_out8(KEYBO_COMMAND,0xd4);			//���������KEYBO_COMMAND�˿����0xd4ʱ��һ�����ݻ��Զ����͸����
	waitkbc();
	io_out8(KEYBO_DATA,0xf4);			//���������KEYBO_DATA�˿������꼤����0xf4���յ�����ָ���ᷢ��0xfa��cpu��Ϊ��
}

void dealkeybored(unsigned char i)				//�����жϴ����ӳ���
{
	char * s=0;						//����ָ���Ҫ������丳ֵ����Ȼ�ᱨ��ֱ��ʹ���з���
	char*keytable=0;
	char leds;
	struct tasks*task=winnow->fifo->task;			//��ѡ�д���ͼ���д����task��ֵ
	leds=(info->leds>>4)&7;					//���ڴ���ȡ��leds��ֵ
	switch(i)
	{
	case 0x0e:							//����������˸��
		if(winnow->fifo)
			writebuf(winnow->fifo,8);break;				//֪ͨѡ�еĴ��ڽ�������	
	case 0x3a:info->leds=(leds^=4)<<4;break;			//CapsLockΪleds�ĵ���λ���޸�������������//������޸ľ��ȸ�leds��ֵȻ����޸ĺ��ֵд��ȥ
	case 0x45:info->leds=(leds^=2)<<4;break;			//NumLockΪleds�ĵ���λ
	case 0x46:info->leds=(leds^=1)<<4;break;			//ScrollLockΪleds�ĵ���λ
	case 0x2a:							//shift��
	case 0x36:shift=~shift;break;					//shift��
	case 0x1c:							//�س���
		if(winnow->fifo)
			writebuf(winnow->fifo,10);break;			//֪ͨѡ�еĴ��ڽ�������
	case 0x53:if((shift!=0x0f)&&(task->tss.ss0!=0))			//����shift+deleteʱǿ�ƹر�Ӧ�ó��򣨸���ss0���鵱ǰ�Ƿ�������Ӧ�ó���
		{
			putstr0("		Application shut down\n");
			io_cli();					//�޸ļĴ���ʱ�����ж�
			task->tss.eax=(int)&(task->tss.esp0);	//endapp��Ҫ�Ĳ���
			task->tss.eip=(int)&endapp;		//���ڴ�����̳���������в���ͬһ�����񣬹ʲ���ͨ�����õķ�ʽ����Ȼ����Ӹ�����ֱ������������������������Ļ���
			io_sti();				//����ֱ���޸������������eip�����������������ת�������������Ż����endapp���ָ���startapp�������״̬��
			top(task->conc.sheet);			//�ر�Ӧ�ó���󷵻ص��øó�������������񲢽����ö�
		}break;
	case 0x1d:initconsole();break;					//����ctrl�����µ������д���
	default:
		keytable=(shift==0x0f)?keytable_n:keytable_s;		//���shift��Ϊԭʼ״̬����keytable_n�еķ��ŷ������keytable_s
		if(keytable[i])
		{
			s[0]=keytable[i];				//����ͨ��s���ɲ�Ȼ���keytable�е�Ԫ����Ϊ��ַ����ȡ��ַ�����Ӷ�����
			if(('A'<=s[0]&&s[0]<='Z')&&(!(leds&4)^(shift!=0x0f)))		//�����д��������shiftͬʱ��������ĸ��ת��ΪСд
				s[0]|=0x20;		
			if(winnow->fifo)
				writebuf(winnow->fifo,s[0]+KEYBOFIFO);	//֪ͨѡ�еĴ��ڽ�������
		}
	}
}

void dealmouse(unsigned char*data,unsigned char i)		//����жϴ����ӳ���
{	
	int mx,my,height,x,y;							//���ľ���λ�ã�ѡ�е�ͼ��߶ȣ���������ѡ��ͼ���λ��	
	struct tasks*task;							//�����жϹرյ��Ƿ���Ӧ�ó��򴰿�
	struct conchar*conc=&winnow->fifo->task->conc;				//��ѡ�д��ڹ�����concΪ�丳ֵ
	switch(variable.mark)							//������������Ϣ����������ʾ����
	{
	case 1:data[0]=i;
		if((data[0]&0xc8)!=0x08)					// !=����������&���ʲ�������ʱ�����Ὣ���һλ��1����������
			variable.mark--;					//�����һ�����ݲ����ڹ涨�ķ�Χ��˵�����ݴ�λ��������������
		break;
	case 2:data[1]=i;
		break;
	case 3:	data[2]=i;
		variable.mark=0;					//�ָ�mark�����׼����һ�ν�������
		
		variable.btn=data[0]&0x07;				//ֻȡ����һ�ĵ���λ��Ϊ��갴��״̬
		variable.x=data[1];					//���ݶ�Ϊx
		variable.y=data[2];					//������Ϊy
		if(data[0]&0x10)					//��������һ����λ��x��y��������
 			variable.x|=0xffffff00;
		if(data[0]&0x20)
			variable.y|=0xffffff00; 
		variable.y=-variable.y;				//�������Ļ��y�����෴(���»���ʱ����յ������λ��Ϊ������������ʾy������˵λ��Ϊ��)			
			
/* 		if(variable.btn&0x01)					//��һλ������˵���������
			s[0]='L';			//�����ɿ���ťʱ������ִ��һ��sprintf��䣬����ʱbtnΪ0�ʲ���ת��Ϊ��д��������ɿ���Сд��Ч��
		if(variable.btn&0x02)					//�ڶ�λ������˵���Ҽ�����
			s[2]='R';
		if(variable.btn&0x04)					//����λ������˵�����ְ���
			s[1]='C'; */
			
		mx=shectl->sheet[MOUSE]->x+variable.x;			//(mx,my)Ϊ���ľ��Ե�ַ
		my=shectl->sheet[MOUSE]->y+variable.y;
		if(mx<0)								//ȷ����겻����Ļ��ʾ�߽�
			mx=0;
		if(mx>info->xnumber-1)
			mx=info->xnumber-1;
		if(my<0)
			my=0;
		if(my>info->ynumber-1)
			my=info->ynumber-1;
		slide(shectl->sheet[MOUSE],mx,my);					//�ڵ�ǰλ��������ʾ���
		if(variable.btn&0x01)							//������������
		{
			if(modol==0)							//��������ƶ�ģʽ
			{
				height=shectl->map[mx-1+my*info->xnumber];		//�жϵ�ǰ���λ��-1���ص�λ�������ĸ��߶ȵ�ͼ��
				if(height!=shectl->top)					//����������棨����������꣩
				{
					top(shectl->sheetaddr[height]);			//����ͼ���ö�
					x=mx-winnow->x;					//��������ͼ�����Ͻǵ�����
					y=my-winnow->y;
					if(winnow->xsize-21<=x&&x<winnow->xsize-5&&5<=y&&y<19)	//�������˹رհ�ť
					{
						task=winnow->fifo->task;			//���봰�ڻ������󶨵�task����ȥ
						if((task->appsheet.num==1)||(task->appsheet.num>0&&winnow==conc->sheet))	//����رյ���Ӧ�ó������һ�����ڻ���������Ӧ�ó���������д��ھ͵����˳�������
						{
							io_cli();				//�޸ļĴ���ʱ�����ж�
							task->tss.eax=(int)&(task->tss.esp0);	//endapp��Ҫ�Ĳ���
							task->tss.eip=(int)&endapp;		//���ڴ�����̳���������в���ͬһ�����񣬹ʲ���ͨ�����õķ�ʽ����Ȼ����Ӹ�����ֱ������������������������Ļ���
							io_sti();				//����ֱ���޸������������eip�����������������ת�������������Ż����endapp���ָ���startapp�������״̬��
							top(task->conc.sheet);			//�ر�Ӧ�ó���󷵻ص��øó�������������񲢽����ö�
						}		
						else 					//�����һ��Ĵ��ھ��ͷŸ�ͼ��
						{
							if(winnow->fifo)			//����ô����л�����
								writebuf(winnow->fifo,KILL);	//֪ͨ�ô���Ҫ���ͷţ������������ȼ�����������fifoд�����ݺ󲻻������л��������񣩣�Ϊ���ܹ��ͷż�ʱ���ȿռ䣬��֪ͨ�������ɸ����������ش������ͨ��task_a�ͷ�����
							else					//�������ͨ����
							{
								free(winnow->buf,winnow->xsize*winnow->ysize);	//�ͷŸ�ͼ���buf�ռ�//ֻ��bufͼ�񻺳�������ʱ����4k�ڴ���䷨ 
								shefree(winnow);				//�ͷŸ�ͼ����ռ�ռ�
							}								
						}
								
					}
					if(3<=x&&x<winnow->xsize-3&&3<=y&&y<21)			//���������ڴ��ڵı�����������϶�
						modol=1;					//�л��������϶�ģʽ						
				}
				else								//������������	
				{
					winnow=shectl->sheet[DESK];				//�����浱Ϊ��ǰѡ���ͼ��
					if(shectl->top>1)					//����д���
						wintitle(shectl->sheetaddr[1],1);		//���ո����ö�״̬��ͼ��������û�		
					refresh(shectl->sheetaddr[1]->x+3,shectl->sheetaddr[1]->x+shectl->sheetaddr[1]->xsize-3,shectl->sheetaddr[1]->y+3,shectl->sheetaddr[1]->y+3+18);
					if(shectl->sheetaddr[1]->fifo)				//����ոյĴ����а󶨻�������֪ͨ������
						writebuf(shectl->sheetaddr[1]->fifo,-2);
				}
			}
			else									//������϶�ģʽ
				slide(winnow,winnow->x+variable.x,winnow->y+variable.y);			
		}
		else										//�ɿ�������뿪�϶�ģʽ
			modol=0;
	}	
	variable.mark++;
}

