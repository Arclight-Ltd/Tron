#include"init.h"
#include"io.h"
#include<stdio.h>

extern struct shectls*shectl;

struct timers*multimer;							//定义任务切换计时器
struct taskctls*taskctl=(struct taskctls*)0x003d2100;			//任务管理表

void initmultask(void)						//初始化任务切换子程序
{
	unsigned char l;
	struct segment_descriptor *gdt = (struct segment_descriptor *) ADR_GDT;
	struct tasks*task_a,*solder;					//定义任务a和哨兵任务
	multimer=timeralloc(2,0,0);					//设置任务切换计时器
	taskctl->sele=4;						//定义初始化段号为4
	
	for(l=0;l<RANK;l++)						//初始化除rank0外的任务链表头和任务数量为0
	{
		taskctl->rank[l].taskhead=0;					
		taskctl->rank[l].num=0;
	}
	task_a=allocbyte(sizeof(struct tasks));				//设置当前任务为任务a，只需设置较少变量，故无需调用taskalloc函数
	task_a->tss.ldtr=0;
	task_a->tss.iomap=0x40000000;					//所有任务的tss必须进行的设置
	task_a->sel=3*8;						//设置任务a的段号为3
	task_a->next=task_a;						//将任务a接到任务a后（建立环状链表）
	task_a->status=WAKE;						//唤醒任务a
	task_a->priority=2;						//任务a的时长为20毫秒
	task_a->rank=0;						//设定任务a为最高优先级
	taskctl->rank[task_a->rank].taskhead=task_a;			//将最高优先级任务链表头指向任务a
	taskctl->taskhead=task_a;					//将当前任务头指向任务a
	taskctl->rank[task_a->rank].num=1;				//当前有一个任务醒着
	setseg(gdt + 3, 103, (int)&task_a->tss, TSS32);	        //将段号为3的任务a的tss登记到GDT
	io_loadtr(3*8);						      //通知CPU当前进行的任务的段号为3（必须在段号3登记到GDT后才能进行此步）
									//任务切换时tr寄存器会自动变化（自动记录当前运行的任务的段号）	
	solder=taskalloc(&soldermain);					//分配哨兵任务
	solder->rank=RANK-1;						//设置哨兵为最低优先级
	taskwake(solder);						//启动哨兵任务								
	
	timeron(multimer);						//开启任务切换计时器
}

struct tasks*taskalloc(void(*main)())				//分配任务空间子程序
{		//该任务指向的主程序地址
	struct segment_descriptor *gdt = (struct segment_descriptor *) ADR_GDT;
	struct tasks*task;
	task=allocbyte(sizeof(struct tasks));				//获得任务空间
	task->sel=taskctl->sele*8;					//设置任务跳转时用的段号（任务段*8）
	inittss32(&task->tss,(int)main);				//初始化该任务的tss
	task->status=SLEEP;						//初始化状态为sleep
	task->priority=2;					    //初始化任务时长为20毫秒（默认为0.02秒，需要改变时再改变task的priority）
	task->rank=1;							//默认优先级为1
	task->next=task;						//默认新任务都是封闭的链表
	task->appsheet.num=0;					//初始化该任务运行app的窗口个数为0（放在这里是为了防止mouse.c中点击关闭按钮时产生误判）
	task->apptimer.num=0;						//初始化该任务运行app的计时器个数为0
	
	setseg(gdt + taskctl->sele, 103, (int)&task->tss,TSS32);	//将该任务的tss登记在GDT中（登记时直接用段号）
	setseg(gdt + taskctl->sele+1000, 15, (int)task->ldt,LDT);	//登记GDT的同时将LDT也登记上去（只需要设置两个段故只取LDT中的16字节，将GDT中的这个段用作LDT信息的储存空间（作为LDT的第X号段））
										//需要LDT段的时候通过GDT寻找相应的LDT段信息，然后再将该LDT段中储存的段首地址赋值过去。//LDT的内存地址通过在GDT中创建LDT段来告知CPU（LDT没有独立的表，它依附在GDT中）（不能同时使用2个以上的LDT）
	taskctl->sele++;						//（利用管理表中的段号登记完后将）任务管理表中的段号+1
	return task;							//返回task的地址
}

void inittss32(struct tss32*tss,int main)		//初始化任务状态段（属于内存段的一种）（需要在GDT中定义后使用）
{		//tss首地址	//该任务要执行的主程序
	int sp;							//任务切换时先备份当前任务的tss，然后用新任务tss中的寄存器设置当前寄存器
	sp=(int)alloc(64*1024);					//分配64KB的栈
	tss->esp1=sp;
	tss->ss0=0;						//没有应用程序运行时保证ss0=0（为mouse.c中shift+F1强制关闭应用程序准备）
	tss->eip=main;						//跳转到另一个任务时会记录当前的ip，下一次跳转回来时从记录的ip开始执行
	tss->eflags=0x202;					//相当于io_sti()
	tss->eax=0;
	tss->ecx=0;
	tss->edx=0;
	tss->ebx=0;
	tss->esp=sp+64*1024;					//指定栈顶
	tss->ebp=0;
	tss->esi=0;
	tss->edi=0;
	tss->es=1*8;						//设为GDT的1号段（整个内存段）
	tss->cs=2*8;						//设为GDT的2号段（内核指令段）						
	tss->ss=1*8;						//32位模式中段寄存器并不指向实际的物理地址
	tss->ds=1*8;						//而是指向GDT的段，根据GDT的段来指定地址
	tss->fs=1*8;
	tss->gs=1*8;
	tss->ldtr=(taskctl->sele+1000)*8;			//将LDT编号储存在ldtr中（任务切换时通知CPU该任务使用的是哪一个LDT）
	tss->iomap=0x40000000;					//ldtr和iomap在切换时不会被CPU赋值，但随意赋值将会导致任务切换失败
}

void taskwake(struct tasks*task)			//唤醒任务子程序（登记任务到环状链表子程序）
{	//任务地址
	struct tasks*header;
	if(task->status==SLEEP)
	{
		if(taskctl->rank[task->rank].num==0)			//如果该级别的任务链表中没有任务
			taskctl->rank[task->rank].taskhead=task;	//就直接成为链表头
		else		
		{
			header=taskctl->rank[task->rank].taskhead;	//将该任务的地址插入链表头和链表头下一个结构体之间
			task->next=header->next;			
			io_cli();
			header->next=task;				//将该任务接入链表
		}						
		io_cli();						//为防止混乱在任务接入链表前先屏蔽中断
		task->status=WAKE;					//由于中断中要用到status属性故要先屏蔽中断再将任务接入
		taskctl->rank[task->rank].num++;			//该等级链表中醒着的任务+1
		io_sti();
	}
	
}

void tasksleep(struct tasks*task)			//休眠任务子程序（将任务从链表中删除）
{	//任务地址						//注意：保持各链表头指向醒着的任务，总表头指向正在执行的任务
	unsigned char c;				
	unsigned long num=0;
	struct tasks*header,*next;
	for(c=0;c<3;c++)						//如果总共有一个以上的任务醒着才让该任务休眠
		num+=taskctl->rank[c].num;
	if((task->status==WAKE)&&(num>=2))				//如果该任务原本是醒着的才让该任务休眠
	{								
		header=taskctl->rank[task->rank].taskhead;		//取出当前任务链表头
		next=header->next;					//next指向下一任务
		while(next!=task)					//查找该任务的位置（此时不需要屏蔽中断）
		{
			header=next;
			next=header->next;
		} 							//当next指向该任务时break	
		io_cli();						//先屏蔽中断		
		if(taskctl->rank[task->rank].num==1)			//如果该任务是该等级链表中唯一的任务
			taskctl->rank[task->rank].taskhead=0;		//直接将链表头设为0
		else
			header->next=task->next;			//把该任务的前一个任务的表尾接上该任务下一个任务的表头（从表中除去）
		if(taskctl->rank[task->rank].taskhead==task)		//如果该任务是所在等级的链表头就将该等级的表头指向表中下一个任务
			taskctl->rank[task->rank].taskhead=task->next;	//表头指向的任务被休眠时要及时更新表头状态使其指向醒着的任务
		task->status=SLEEP;					//设置该任务的状态为休眠
		taskctl->rank[task->rank].num--;			//该级别链表醒着的任务-1
		task->next=task;					//将该任务复原为封闭状态（每个任务休眠时都要恢复为该状态）
		if(taskctl->taskhead==task)				//如果该任务是当前正在执行的任务
		{
			c=0;
			while(!taskctl->rank[c].num)			//寻找最高优先级的任务表头
				c++;
			taskctl->taskhead=taskctl->rank[c].taskhead;	//时刻保持taskctl->taskhead指向当前任务（表头都是要执行的任务）
			io_jmpfar(0,taskctl->taskhead->sel);
		}
		io_sti();						//跳转回来时立刻打开中断
	}
}

void taskrank(struct tasks*task,unsigned char rank,unsigned char priority)	//修改任务等级子程序
{		//要改变的任务//新的等级	//新的执行时间
	struct tasks*header,*next,*oldnext;
	unsigned char oldrank,oldstatus,c=0;
	oldstatus=task->status;						//保存任务原来的属性
	oldrank=task->rank;
	oldnext=task->next;
	io_cli();							//改变任务操作，先屏蔽中断
	task->priority=priority;					//赋予新值
	task->rank=rank;
	if(oldrank!=rank)					//如果等级发生了改变就在新链表中添加该任务
	{
		if(taskctl->rank[task->rank].num==0)			//如果该级别的任务链表中没有任务
			taskctl->rank[task->rank].taskhead=task;	//就直接成为链表头
		else		
		{
			header=taskctl->rank[task->rank].taskhead;	//将该任务的地址插入链表头和链表头下一个结构体之间
			task->next=header->next;			
			header->next=task;				//将该任务接入链表
		}						
		task->status=WAKE;					//由于中断中要用到status属性故要先屏蔽中断再将任务接入
		taskctl->rank[task->rank].num++;			//该等级链表中醒着的任务+1
	}
	if((oldstatus==WAKE)&&(oldrank!=rank))			//如果该任务原本在链表内并且等级发生了改变才需要将其从原链表中删除
	{								
		header=taskctl->rank[oldrank].taskhead;			//取出当前任务链表头
		next=header->next;					//next指向下一任务
		while(next!=task)					//查找该任务的位置
		{
			header=next;
			next=header->next;
		} 							//当next指向该任务时break		
		if(taskctl->rank[oldrank].num==1)			//如果该任务是原等级链表中唯一的任务
			taskctl->rank[oldrank].taskhead=0;		//直接将链表头设为0
		else						//由于上一步将该任务接入了新的链表，该任务的next已经变化，故要用oldnext
			header->next=oldnext;				//把该任务的前一个任务的表尾接上该任务下一个任务的表头		
		if(taskctl->rank[task->rank].taskhead==task)		//如果该任务是原等级的链表头就将原等级的表头指向表中下一个任务
			taskctl->rank[task->rank].taskhead=oldnext;	//如果该任务既是表头也是唯一任务，由于前面已经设表头为0故不会执行此步		
		taskctl->rank[oldrank].num--;				//原级别链表醒着的任务-1(由于不是休眠任务故不需要复原为封闭状态)	
		if(taskctl->taskhead==task)				//如果该任务是当前正在执行的任务
		{
			while(!taskctl->rank[c].num)			//寻找最高优先级的任务表头
				c++;
			taskctl->taskhead=taskctl->rank[c].taskhead;	//时刻保持taskctl->taskhead指向当前任务
			io_jmpfar(0,taskctl->taskhead->sel);
		}
	}
	io_sti();							//跳转回来时立刻打开中断
}

void switask(void)					//切换任务子程序
{
	unsigned char c=0,ts=0;
	while(!taskctl->rank[c].num)					//寻找最高优先级的任务表头
		c++;	
	if(taskctl->taskhead!=taskctl->rank[c].taskhead->next)		//如果当前正在执行的任务和表头的下一任务（跳转目的）不一样才跳转
		ts=1;
	taskctl->taskhead=taskctl->rank[c].taskhead;			//将当前正在执行的任务指向表头
	multimer->timeout=taskctl->taskhead->next->priority;		//用下一个任务的时长设置任务切换计时器
	timeron(multimer);						//重新开启任务切换计时器
	if(ts)								//当要执行的任务数大于等于2时进行任务切换（只有一个时不切换）
	{								//只有一个任务时进行任务切换CPU会拒绝执行该指令而使程序乱套
		taskctl->taskhead=taskctl->taskhead->next;		//将任务链表头指向下一个任务
		taskctl->rank[c].taskhead=taskctl->taskhead;		//保持该优先级链表头及时更新以免下一次切换时跳转到相同的任务引发崩溃
		io_jmpfar(0,taskctl->taskhead->sel);			//跳转到该任务指向的段开始执行
	}   
}

void soldermain(void)					//当没有任务要执行时就执行哨兵任务，避免任务不能休眠并且降低机器负荷
{
	while(1)
		io_hlt();
}
