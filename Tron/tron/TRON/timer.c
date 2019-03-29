#include"io.h"
#include"init.h"

struct timerctls*timerctl=(struct timerctls*)0x003d2000;

struct timers*timeralloc(unsigned int timeout,char data,struct fifo32*fifo)			//����ʱ�������ڴ�ռ��ӳ���
{			//Ҫ������ʱ��      //�ü�ʱ���ı��//�ü�ʱ���Ļ�����
	struct timers*timerx;
	timerx=allocbyte(sizeof(struct timers));			//��ȡ�ռ�		
	timerx->timeout=timeout;					//���ü�ʱ��������
	timerx->data=data;
	timerx->next=0;		
	timerx->fifo=fifo;
	timerx->status=TIMEROFF;
	return timerx;								
}
						
void timerfree(struct timers*timerx)				//��ʱ���ڴ��ͷ��ӳ���
{	//��ʱ����ַ							//��������ʱ����ʹ�������ṹ��ָ����ʹ���򷽱�
	struct timers*pre=timerctl->timerhead,*c;
	for(c=timerctl->timerhead;c!=0;pre=c,c=c->next)				//����ʱ�����������޸ü�ʱ��					
		if(c==timerx)							//�����
		{
			if(pre==c)						//���������ͷ
			{
				io_cli();
				timerctl->timerhead=timerctl->timerhead->next;	//������ͷָ��ԭ��ͷ������һ����ʱ������ֻ��һ����ʱ����ָ��0��
				io_sti();
				freebyte(timerx,sizeof(struct timers));		//�ͷŸü�ʱ�����ڴ�
				return;
			}
			pre->next=c->next;					//����Ǳ����򽫸ü�ʱ���������г���
			freebyte(timerx,sizeof(struct timers));
			return;
		}	
}
						
void timeron(struct timers*timerx)				//������ʱ���ӳ�����취�����������ж�ʱ���еĴ���
{	//��ʱ����ַ					
	struct timers*pre,*c;							//��Ϊ���ڱ���׹ʲ����ܳ���û������ͷ��Ҫ�ӵ�����β�����
	int eflag;
	if(timerx->status==TIMEROFF)					//����ü�ʱ��ԭ���ǹ��ŵĲ�ִ�п������򣨱�������������ѭ����
	{
		eflag=io_popeflag();					//ΪʲôҪ���ݱ�־�Ĵ�����
		io_cli();							//��һ�ڼ�����жϾͻ�������ҹ�Ҫ��һ��ͷ�������ж�
		pre=timerctl->timerhead;					//preָ�����ڹ����ļ�ʱ��������ͷ
		timerx->counter=timerctl->counter + timerx->timeout;		//�ü�ʱ����counter=��ǰ������counter+�ü�ʱ������ʱ��
		for(c=timerctl->timerhead;c!=0;pre=c,c=c->next)			//����ʱ������		
			if(timerx->counter <= c->counter)			//����ҵ�һ��ԤԼʱ���timerx���
			{
				timerx->status=TIMERON;				//��ʱ��Ϊ����״̬
				if(pre==c)					//����Ǳ�����ͷ��Ҫ��
				{				
					timerx->next=timerctl->timerhead;	//ԭ����ͷ����timerx����
					timerctl->timerhead=timerx;		//timerx��Ϊ�µ�����ͷ
					io_pusheflag(eflag);
					return;
				}
				timerx->next=c;					//����Ǳ��������pre�ṹ���c�ṹ����м�
				pre->next=timerx;
				io_pusheflag(eflag);				//�ָ������ж�ǰ�ı�־�Ĵ���ֵ���൱�ڴ��ж�
				return;
			}
	}
	
}

void timersolder(void)						//�����ڱ��ӳ���
{
	struct timers*solder;
	solder=timeralloc(0xffffffff,0xff,0);			//����һ���ڱ�һֱ���ڼ�ʱ�������У�ѹ��ף��������û�������
	solder->counter=0xffffffff;
	timerctl->timerhead=solder;				//��ʼ������ͷָ���ڱ������ڳ�ʼ���ڴ�����ǰ�����ڱ����ڱ��ᱻ��գ�
	solder->status=TIMERON;					//���ڱ���ʱ��
}
