#include<stdio.h>
#include"io.h"
#include"init.h"

extern struct fifo32 mainfifo;
extern struct fifo32 mousefifo;
extern struct timerctls*timerctl;
extern struct timers*multimer;
extern struct taskctls*taskctl;


/****************************************************�쳣�������ж�**********************************************************************/

int*int_0dh(int *esp)							//һ�㱣���쳣�жϱ������ӳ���
{
	char*s=0;
	struct tasks *task = taskctl->taskhead;
	putstr0("		General Protected Exception.\n");		//��ʾ��ʾ��
	sprintf(s,"			EIP=%08X\n",esp[11]);		//esp��ֵ��intfunc.nasѹ��ջ�С�һ���β�ֻ�ܴ�ջ��ȡ��һ�����ݡ��˴��Ὣ[esp+4]������ȡ����������ַ����esp�����µĲ���ֻ�ܶԸõ�ַ���ж��޷���ȡ��[esp+4]�Ժ������
	putstr0(s);							//����ʱջ�����ݵķֲ�����intfunc�еĵ��ó����Ƴ������P451
	return &(task->tss.esp0);						//ǿ�ƽ���Ӧ�ó��򲢷��ص�����ϵͳ
}

int*int_0ch(int *esp)							//ջ�쳣�жϱ������ӳ���
{
	char*s=0;
	struct tasks *task = taskctl->taskhead;
	putstr0("		Stack Exception.\n");		//��ʾ��ʾ��
	sprintf(s,"		  EIP=%08X\n",esp[11]);			//��ʾ����ָ���esp�������Ӧ�ó���.hrb��esp��
	putstr0(s);
	return &(task->tss.esp0);						//ǿ�ƽ���Ӧ�ó��򲢷��ص�����ϵͳ
}

/*****************************************************IRQ�ж�****************************************************************************/

void int_20h(int *esp)						//0�Ŷ�ʱ���жϱ������ӳ���
{
	char ts=0;
	io_out8(PIC0_OCW2, 0x60);						//֪ͨ��PIC�Ѿ����յ��ж�֪ͨ��ʹPIC��������IRQ0���ж����
	timerctl->counter++;								//��counter��һ
check:	if(timerctl->counter==timerctl->timerhead->counter)		//ֻ�Ƚ�����ͷ��ʱ����ԤԼʱ���Ƿ񵽴� �Խ�ʡ�ж�����ʱ��
	{	
		if(timerctl->timerhead==multimer)
			ts=1;
		else
			writebuf(timerctl->timerhead->fifo,timerctl->timerhead->data);		//���ü�ʱ���ı���͵��ü�ʱ����buf��
		timerctl->timerhead->status=TIMEROFF;			//�رոü�ʱ��
		timerctl->timerhead=timerctl->timerhead->next;		//������ʱ������ͷָ����һ��ԤԼ�ļ�ʱ��
		goto check;						//���¼���µı�ͷ�Ƿ񵽴�ԤԼʱ���Է�ֹ���ԤԼʱ������һ����ͬ�ļ�ʱ��
	}
	if(ts)
		switask();
}

void int_21h(int*esp)						//1�ż����жϱ������ӳ���
{
	io_out8(PIC0_OCW2, 0x61);					//֪ͨ��PIC�Ѿ����յ��ж�֪ͨ��ʹPIC��������IRQ1���ж����
	writebuf(&mainfifo,io_in8(KEYBO_DATA)+KEYBOFIFO);		//�Ӽ��̶˿ڻ�ȡ���¼���ascll�벢������buf��
}

void int_27h(int *esp)						//7���жϱ������ӳ���
{									
	io_out8(PIC0_OCW2, 0x67);					//֪ͨPIC�Ѿ����յ��ж�֪ͨ��ʹPIC��������IRQ7���ж����
									/* PIC0����β���ȫ����z�ߌ��� 
									 Athlon64X2�C�ʤɤǤϥ��åץ��åȤζ��Ϥˤ��PIC�γ��ڻ��r�ˤ��θ���z�ߤ�1�Ȥ��������� 
									 ���θ���z�߄I���v���ϡ����θ���z�ߤˌ����ƺΤ⤷�ʤ��Ǥ���^���� 
									 �ʤ��Τ⤷�ʤ��Ƥ����Σ�
									���θ���z�ߤ�PIC���ڻ��r��늚ݵĤʥΥ����ˤ�äưk��������ΤʤΤǡ�
									�ޤ���˺Τ��I���Ƥ���Ҫ���ʤ���		*/						
}

void int_2ch(int*esp)						//12������жϱ������ӳ���
{
	io_out8(PIC1_OCW2, 0x64);					//֪ͨ��PIC�Ѿ����յ��ж�֪ͨ��ʹPIC��������IRQ12���ڴ�PIC�к���Ϊ4�����ж����
	io_out8(PIC0_OCW2, 0x62);					//֪ͨ��PIC�Ѿ����յ��ж�֪ͨ��ʹPIC��������IRQ2����PIC�����ж����
	writebuf(&mousefifo,io_in8(KEYBO_DATA));			//�Ӽ��̶˿ڻ�ȡ��괫�������ݲ�������buf��
}

/**********************************************************API***************************************************************************/


int *int_40h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{								//�ַ�����ʾAPI
	int i;
	int *reg;
	int filestackbase = *((int *) 0xfe8);				//Ӧ�ó������ڴ���ʱ�׵�ַ
	struct tasks *task =taskctl->taskhead;	
	switch(edx)
	{
	case 1:putchar(eax & 0xff);break;				//һ�Ź��ܣ���ʾ�����ַ���eax=Ҫ��ʾ�ַ���
	case 2:putstr0((char *)ebx+filestackbase);break;		//���Ź��ܣ���ʾ�ַ������ԡ�\0��������ebx=�ַ����׵�ַ��
	case 3:putstr1((char *)ebx+filestackbase,ecx);break;		//���Ź��ܣ���ʾָ�����ȵ��ַ�����ebx=�ַ����׵�ַ��ecx=�ַ�������
	case 4:	return &(task->tss.esp0);				//�ĺŹ��ܣ���Ӧ�ó��򷵻ص�����ϵͳ
	case 5:	reg=&eax+1;						//��Ź��ܣ���ȡ����������
		while(!bufstatus(&task->appsheet.fifo));		//���û�����ݾ�һֱ�ȵ�������	
		io_cli();
		i=readbuf(&task->appsheet.fifo);			//���������󶨵�app�������ж�ȡ����					
		io_sti();			
		reg[7]=i;			
	}
	return 0;
}

int *int_41h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{								//������ʾAPI
	int filestackbase = *((int *) 0xfe8);				//Ӧ�ó������ڴ���ʱ�׵�ַ
	int *reg;
	struct sheets*appwin;						//Ӧ�ó���Ĵ���
	struct tasks *task =taskctl->taskhead;	
	switch(edx)
	{
	case 1:	reg=&eax+1;						//һ�Ź��ܣ�����Ӧ�ó��򴰿� eax��ַ�ĺ�һλ�൱��espָ��eax�ĺ�һλ
		appwin=shealloc((char*)ebx+filestackbase,esi,edi,eax);		//���漰Ӧ�ó�������ݵ�ַ��Ҫ+filestackbase
		task->appsheet.sheet[task->appsheet.num]=appwin;		//�Ǽ�����ͼ����е�ͬʱҲ�Ǽ���appר�õ�ͼ���
		task->appsheet.num++;						//�Ǽ���Ӧ�ó����õ�ͼ��������
		appwin->fifo=&task->appsheet.fifo;				//��task���Ѿ�����õĻ�������ֵ��app�õĻ�����
		displaywindow(appwin,(char*)ecx+filestackbase,1);		//ebx=��������esi=xsize��edi=ysize��eax=͸��ɫ��ecx=��������
		slide(appwin,200,100);
		top(appwin);
		reg[7]=(int)appwin;						//reg[0]=edi������������reg[4]=ebx������������reg[7]=eax   ��ͼ��ṹ���ַ��Ϊ����ֵ
		break;
	case 2:	appwin=(struct sheets*)ebx;				//���Ź��ܣ��ڴ�������ʾ�ַ���
		displaywords((char*)(ebp+filestackbase),esi,edi,ecx,appwin,eax,0);break;
	case 3:	appwin=(struct sheets*)ebx;				//���Ź��ܣ��ڴ�������ʾ����
		squer(eax,esi,ecx,edi,appwin->buf,appwin->xsize,ebp);
		refresh(appwin->x+eax,appwin->x+esi,appwin->y+ecx,appwin->y+edi);break;
	case 4:	shefree((struct sheets*)ebx);break;			//�ĺŹ��ܣ��رմ��ڣ�Ӧ�ô����ͷų��򼯳ɵ���shefree�����У�
	}
	return 0;
}

int *int_42h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{								//�ڴ����API
	int filestackbase = *((int *) 0xfe8);
	struct manage*mem=(struct manage*)(ebx+filestackbase);		//ebx=�ڴ�������Ӧ���ļ�������׵�ַ
	unsigned int c,d;						//�ڴ�������
	void *addr;							//�ݴ��׵�ַ	
	int*reg=&eax+1;	
	switch(edx)
	{
	case 1:	mem->block=1;					//����һ����ʼ��Ӧ�ó����ڴ�������32KB��
		mem->freememo=ecx;					//ecx=�ڴ��С
		mem->lost=0;	
		mem->lostsize=0;	
		mem->memo[0].addr=(void*)eax;				//����eax�Ǹ�Ӧ�ó���ʹ�õ����Բ���Ҫ+filestackbase
		mem->memo[0].size=ecx;break;					//eax=�����ڴ�ռ����ʼ��ַ
	case 2:	for(c=0;c<mem->block;c++)			//���ܶ��������ڴ�ռ�	
			if(ecx<=mem->memo[c].size)			//�����  ��ecx=���������ֽ�����
			{
				addr=mem->memo[c].addr;			//�����ڴ���׵�ַ����addr
				mem->memo[c].addr+=ecx;			//���ڴ���׵�ַ����
				mem->memo[c].size-=ecx;			//�ڴ��Ŀ����ڴ��С����
				mem->freememo-=ecx;			//�ܿ����ڴ����
				if(!mem->memo[c].size)			//������ڴ���СΪ0
				{
					mem->block--;			//ȥ�����ڴ��
					for(;c<mem->block;c++)		//���ڴ��֮����ڴ��ǰ�Ƹ���
						mem->memo[c]=mem->memo[c+1];				
				}
				reg[7]=(int)addr;break;			//�����׵�ַ��eax=���䵽���ڴ��ַ��
			}
	case 3:	for(c=0;c<mem->block;c++)			//���������ͷ��ڴ�ռ�
			if(eax<(int)mem->memo[c].addr)			//eax=��Ҫ�ͷŵ��ڴ�ռ��׵�ַ
				break;
		if(c>0)								//��֤[c-1]��Խ��
			if(mem->memo[c-1].addr+mem->memo[c-1].size==(int*)eax)  	//��Ҫ�ͷŵ��׵�ַ��ǰ��һ���ڴ��������һ��ʱ
			{
				mem->memo[c-1].size+=ecx;				//ǰһ�ڴ������ڴ��С���ӣ�ecx=�ͷſռ�Ĵ�С��
				mem->freememo+=ecx;					//�ܿ����ڴ�����
				if((int*)eax+ecx==mem->memo[c].addr)			//���Ҫ�ͷŵ��׵�ַ�����һ�ڴ��������һ��
				{
					mem->memo[c-1].size+=mem->memo[c].size;	//����һ�ڴ��Ĵ�С���ӵ�ǰһ�ڴ����
					mem->block--;				//�ڴ����������
					for(;c<mem->block;c++)			//�ڴ��ǰ�Ƹ���
						mem->memo[c]=mem->memo[c+1];
				}break;					
			}
		if((int*)eax+ecx==mem->memo[c].addr)			//��Ҫ�ͷŵ��׵�ַ������ǰһ�ڴ������һ���ܺͺ�һ���ڴ��������һ��ʱ
		{
			mem->memo[c].addr=(int*)eax;			//��һ�ڴ���׵�ַ��ΪҪ�ͷ��ڴ���׵�ַ
			mem->memo[c].size+=ecx;				//��һ�ڴ������ڴ��С����
			mem->freememo+=ecx;				//�ܿ����ڴ�����
			break;
		}			
		if(mem->block<BLOCKNUM)					//���ǰ�󶼲����������ڴ�������������С��Χ��
		{
			for(d=mem->block;d>c;d--)			//�Ӻ�һ�ڴ�飨������ʼȫ���ڴ�����
				mem->memo[d]=mem->memo[d-1];
			mem->memo[c].addr=(int*)eax;			//��Ҫ�ͷŵ��ڴ����Ϣд�ڡ���һ���ڴ��ԭ����λ��
			mem->memo[c].size=ecx;
			mem->freememo+=ecx;				//�ܿ����ڴ�����
			mem->block++;					//�ڴ����������
			break;
		}
		else							//����ڴ��������
		{
			mem->lostsize+=ecx;				//����ǰҪ�ͷŵ��ڴ���������¼���������ڴ��С
			mem->lost++;					//��¼�����ڴ�Ĵ���
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
