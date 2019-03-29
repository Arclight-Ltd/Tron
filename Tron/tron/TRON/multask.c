#include"init.h"
#include"io.h"
#include<stdio.h>

extern struct shectls*shectl;

struct timers*multimer;							//���������л���ʱ��
struct taskctls*taskctl=(struct taskctls*)0x003d2100;			//��������

void initmultask(void)						//��ʼ�������л��ӳ���
{
	unsigned char l;
	struct segment_descriptor *gdt = (struct segment_descriptor *) ADR_GDT;
	struct tasks*task_a,*solder;					//��������a���ڱ�����
	multimer=timeralloc(2,0,0);					//���������л���ʱ��
	taskctl->sele=4;						//�����ʼ���κ�Ϊ4
	
	for(l=0;l<RANK;l++)						//��ʼ����rank0�����������ͷ����������Ϊ0
	{
		taskctl->rank[l].taskhead=0;					
		taskctl->rank[l].num=0;
	}
	task_a=allocbyte(sizeof(struct tasks));				//���õ�ǰ����Ϊ����a��ֻ�����ý��ٱ��������������taskalloc����
	task_a->tss.ldtr=0;
	task_a->tss.iomap=0x40000000;					//���������tss������е�����
	task_a->sel=3*8;						//��������a�Ķκ�Ϊ3
	task_a->next=task_a;						//������a�ӵ�����a�󣨽�����״����
	task_a->status=WAKE;						//��������a
	task_a->priority=2;						//����a��ʱ��Ϊ20����
	task_a->rank=0;						//�趨����aΪ������ȼ�
	taskctl->rank[task_a->rank].taskhead=task_a;			//��������ȼ���������ͷָ������a
	taskctl->taskhead=task_a;					//����ǰ����ͷָ������a
	taskctl->rank[task_a->rank].num=1;				//��ǰ��һ����������
	setseg(gdt + 3, 103, (int)&task_a->tss, TSS32);	        //���κ�Ϊ3������a��tss�Ǽǵ�GDT
	io_loadtr(3*8);						      //֪ͨCPU��ǰ���е�����Ķκ�Ϊ3�������ڶκ�3�Ǽǵ�GDT����ܽ��д˲���
									//�����л�ʱtr�Ĵ������Զ��仯���Զ���¼��ǰ���е�����Ķκţ�	
	solder=taskalloc(&soldermain);					//�����ڱ�����
	solder->rank=RANK-1;						//�����ڱ�Ϊ������ȼ�
	taskwake(solder);						//�����ڱ�����								
	
	timeron(multimer);						//���������л���ʱ��
}

struct tasks*taskalloc(void(*main)())				//��������ռ��ӳ���
{		//������ָ����������ַ
	struct segment_descriptor *gdt = (struct segment_descriptor *) ADR_GDT;
	struct tasks*task;
	task=allocbyte(sizeof(struct tasks));				//�������ռ�
	task->sel=taskctl->sele*8;					//����������תʱ�õĶκţ������*8��
	inittss32(&task->tss,(int)main);				//��ʼ���������tss
	task->status=SLEEP;						//��ʼ��״̬Ϊsleep
	task->priority=2;					    //��ʼ������ʱ��Ϊ20���루Ĭ��Ϊ0.02�룬��Ҫ�ı�ʱ�ٸı�task��priority��
	task->rank=1;							//Ĭ�����ȼ�Ϊ1
	task->next=task;						//Ĭ���������Ƿ�յ�����
	task->appsheet.num=0;					//��ʼ������������app�Ĵ��ڸ���Ϊ0������������Ϊ�˷�ֹmouse.c�е���رհ�ťʱ�������У�
	task->apptimer.num=0;						//��ʼ������������app�ļ�ʱ������Ϊ0
	
	setseg(gdt + taskctl->sele, 103, (int)&task->tss,TSS32);	//���������tss�Ǽ���GDT�У��Ǽ�ʱֱ���öκţ�
	setseg(gdt + taskctl->sele+1000, 15, (int)task->ldt,LDT);	//�Ǽ�GDT��ͬʱ��LDTҲ�Ǽ���ȥ��ֻ��Ҫ���������ι�ֻȡLDT�е�16�ֽڣ���GDT�е����������LDT��Ϣ�Ĵ���ռ䣨��ΪLDT�ĵ�X�ŶΣ���
										//��ҪLDT�ε�ʱ��ͨ��GDTѰ����Ӧ��LDT����Ϣ��Ȼ���ٽ���LDT���д���Ķ��׵�ַ��ֵ��ȥ��//LDT���ڴ��ַͨ����GDT�д���LDT������֪CPU��LDTû�ж����ı���������GDT�У�������ͬʱʹ��2�����ϵ�LDT��
	taskctl->sele++;						//�����ù�����еĶκŵǼ���󽫣����������еĶκ�+1
	return task;							//����task�ĵ�ַ
}

void inittss32(struct tss32*tss,int main)		//��ʼ������״̬�Σ������ڴ�ε�һ�֣�����Ҫ��GDT�ж����ʹ�ã�
{		//tss�׵�ַ	//������Ҫִ�е�������
	int sp;							//�����л�ʱ�ȱ��ݵ�ǰ�����tss��Ȼ����������tss�еļĴ������õ�ǰ�Ĵ���
	sp=(int)alloc(64*1024);					//����64KB��ջ
	tss->esp1=sp;
	tss->ss0=0;						//û��Ӧ�ó�������ʱ��֤ss0=0��Ϊmouse.c��shift+F1ǿ�ƹر�Ӧ�ó���׼����
	tss->eip=main;						//��ת����һ������ʱ���¼��ǰ��ip����һ����ת����ʱ�Ӽ�¼��ip��ʼִ��
	tss->eflags=0x202;					//�൱��io_sti()
	tss->eax=0;
	tss->ecx=0;
	tss->edx=0;
	tss->ebx=0;
	tss->esp=sp+64*1024;					//ָ��ջ��
	tss->ebp=0;
	tss->esi=0;
	tss->edi=0;
	tss->es=1*8;						//��ΪGDT��1�ŶΣ������ڴ�Σ�
	tss->cs=2*8;						//��ΪGDT��2�ŶΣ��ں�ָ��Σ�						
	tss->ss=1*8;						//32λģʽ�жμĴ�������ָ��ʵ�ʵ������ַ
	tss->ds=1*8;						//����ָ��GDT�ĶΣ�����GDT�Ķ���ָ����ַ
	tss->fs=1*8;
	tss->gs=1*8;
	tss->ldtr=(taskctl->sele+1000)*8;			//��LDT��Ŵ�����ldtr�У������л�ʱ֪ͨCPU������ʹ�õ�����һ��LDT��
	tss->iomap=0x40000000;					//ldtr��iomap���л�ʱ���ᱻCPU��ֵ�������⸳ֵ���ᵼ�������л�ʧ��
}

void taskwake(struct tasks*task)			//���������ӳ��򣨵Ǽ����񵽻�״�����ӳ���
{	//�����ַ
	struct tasks*header;
	if(task->status==SLEEP)
	{
		if(taskctl->rank[task->rank].num==0)			//����ü��������������û������
			taskctl->rank[task->rank].taskhead=task;	//��ֱ�ӳ�Ϊ����ͷ
		else		
		{
			header=taskctl->rank[task->rank].taskhead;	//��������ĵ�ַ��������ͷ������ͷ��һ���ṹ��֮��
			task->next=header->next;			
			io_cli();
			header->next=task;				//���������������
		}						
		io_cli();						//Ϊ��ֹ�����������������ǰ�������ж�
		task->status=WAKE;					//�����ж���Ҫ�õ�status���Թ�Ҫ�������ж��ٽ��������
		taskctl->rank[task->rank].num++;			//�õȼ����������ŵ�����+1
		io_sti();
	}
	
}

void tasksleep(struct tasks*task)			//���������ӳ��򣨽������������ɾ����
{	//�����ַ						//ע�⣺���ָ�����ͷָ�����ŵ������ܱ�ͷָ������ִ�е�����
	unsigned char c;				
	unsigned long num=0;
	struct tasks*header,*next;
	for(c=0;c<3;c++)						//����ܹ���һ�����ϵ��������Ų��ø���������
		num+=taskctl->rank[c].num;
	if((task->status==WAKE)&&(num>=2))				//���������ԭ�������ŵĲ��ø���������
	{								
		header=taskctl->rank[task->rank].taskhead;		//ȡ����ǰ��������ͷ
		next=header->next;					//nextָ����һ����
		while(next!=task)					//���Ҹ������λ�ã���ʱ����Ҫ�����жϣ�
		{
			header=next;
			next=header->next;
		} 							//��nextָ�������ʱbreak	
		io_cli();						//�������ж�		
		if(taskctl->rank[task->rank].num==1)			//����������Ǹõȼ�������Ψһ������
			taskctl->rank[task->rank].taskhead=0;		//ֱ�ӽ�����ͷ��Ϊ0
		else
			header->next=task->next;			//�Ѹ������ǰһ������ı�β���ϸ�������һ������ı�ͷ���ӱ��г�ȥ��
		if(taskctl->rank[task->rank].taskhead==task)		//��������������ڵȼ�������ͷ�ͽ��õȼ��ı�ͷָ�������һ������
			taskctl->rank[task->rank].taskhead=task->next;	//��ͷָ�����������ʱҪ��ʱ���±�ͷ״̬ʹ��ָ�����ŵ�����
		task->status=SLEEP;					//���ø������״̬Ϊ����
		taskctl->rank[task->rank].num--;			//�ü����������ŵ�����-1
		task->next=task;					//��������ԭΪ���״̬��ÿ����������ʱ��Ҫ�ָ�Ϊ��״̬��
		if(taskctl->taskhead==task)				//����������ǵ�ǰ����ִ�е�����
		{
			c=0;
			while(!taskctl->rank[c].num)			//Ѱ��������ȼ��������ͷ
				c++;
			taskctl->taskhead=taskctl->rank[c].taskhead;	//ʱ�̱���taskctl->taskheadָ��ǰ���񣨱�ͷ����Ҫִ�е�����
			io_jmpfar(0,taskctl->taskhead->sel);
		}
		io_sti();						//��ת����ʱ���̴��ж�
	}
}

void taskrank(struct tasks*task,unsigned char rank,unsigned char priority)	//�޸�����ȼ��ӳ���
{		//Ҫ�ı������//�µĵȼ�	//�µ�ִ��ʱ��
	struct tasks*header,*next,*oldnext;
	unsigned char oldrank,oldstatus,c=0;
	oldstatus=task->status;						//��������ԭ��������
	oldrank=task->rank;
	oldnext=task->next;
	io_cli();							//�ı�����������������ж�
	task->priority=priority;					//������ֵ
	task->rank=rank;
	if(oldrank!=rank)					//����ȼ������˸ı��������������Ӹ�����
	{
		if(taskctl->rank[task->rank].num==0)			//����ü��������������û������
			taskctl->rank[task->rank].taskhead=task;	//��ֱ�ӳ�Ϊ����ͷ
		else		
		{
			header=taskctl->rank[task->rank].taskhead;	//��������ĵ�ַ��������ͷ������ͷ��һ���ṹ��֮��
			task->next=header->next;			
			header->next=task;				//���������������
		}						
		task->status=WAKE;					//�����ж���Ҫ�õ�status���Թ�Ҫ�������ж��ٽ��������
		taskctl->rank[task->rank].num++;			//�õȼ����������ŵ�����+1
	}
	if((oldstatus==WAKE)&&(oldrank!=rank))			//���������ԭ���������ڲ��ҵȼ������˸ı����Ҫ�����ԭ������ɾ��
	{								
		header=taskctl->rank[oldrank].taskhead;			//ȡ����ǰ��������ͷ
		next=header->next;					//nextָ����һ����
		while(next!=task)					//���Ҹ������λ��
		{
			header=next;
			next=header->next;
		} 							//��nextָ�������ʱbreak		
		if(taskctl->rank[oldrank].num==1)			//�����������ԭ�ȼ�������Ψһ������
			taskctl->rank[oldrank].taskhead=0;		//ֱ�ӽ�����ͷ��Ϊ0
		else						//������һ����������������µ������������next�Ѿ��仯����Ҫ��oldnext
			header->next=oldnext;				//�Ѹ������ǰһ������ı�β���ϸ�������һ������ı�ͷ		
		if(taskctl->rank[task->rank].taskhead==task)		//�����������ԭ�ȼ�������ͷ�ͽ�ԭ�ȼ��ı�ͷָ�������һ������
			taskctl->rank[task->rank].taskhead=oldnext;	//�����������Ǳ�ͷҲ��Ψһ��������ǰ���Ѿ����ͷΪ0�ʲ���ִ�д˲�		
		taskctl->rank[oldrank].num--;				//ԭ�����������ŵ�����-1(���ڲ�����������ʲ���Ҫ��ԭΪ���״̬)	
		if(taskctl->taskhead==task)				//����������ǵ�ǰ����ִ�е�����
		{
			while(!taskctl->rank[c].num)			//Ѱ��������ȼ��������ͷ
				c++;
			taskctl->taskhead=taskctl->rank[c].taskhead;	//ʱ�̱���taskctl->taskheadָ��ǰ����
			io_jmpfar(0,taskctl->taskhead->sel);
		}
	}
	io_sti();							//��ת����ʱ���̴��ж�
}

void switask(void)					//�л������ӳ���
{
	unsigned char c=0,ts=0;
	while(!taskctl->rank[c].num)					//Ѱ��������ȼ��������ͷ
		c++;	
	if(taskctl->taskhead!=taskctl->rank[c].taskhead->next)		//�����ǰ����ִ�е�����ͱ�ͷ����һ������תĿ�ģ���һ������ת
		ts=1;
	taskctl->taskhead=taskctl->rank[c].taskhead;			//����ǰ����ִ�е�����ָ���ͷ
	multimer->timeout=taskctl->taskhead->next->priority;		//����һ�������ʱ�����������л���ʱ��
	timeron(multimer);						//���¿��������л���ʱ��
	if(ts)								//��Ҫִ�е����������ڵ���2ʱ���������л���ֻ��һ��ʱ���л���
	{								//ֻ��һ������ʱ���������л�CPU��ܾ�ִ�и�ָ���ʹ��������
		taskctl->taskhead=taskctl->taskhead->next;		//����������ͷָ����һ������
		taskctl->rank[c].taskhead=taskctl->taskhead;		//���ָ����ȼ�����ͷ��ʱ����������һ���л�ʱ��ת����ͬ��������������
		io_jmpfar(0,taskctl->taskhead->sel);			//��ת��������ָ��Ķο�ʼִ��
	}   
}

void soldermain(void)					//��û������Ҫִ��ʱ��ִ���ڱ����񣬱������������߲��ҽ��ͻ�������
{
	while(1)
		io_hlt();
}
