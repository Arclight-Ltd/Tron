#include"io.h"
#include"init.h"

extern struct timerctls*timerctl;

void initGDTIDT(void)											//初始化GDT和IDT子程序
{
	struct segment_descriptor *gdt = (struct segment_descriptor *) ADR_GDT;			//GDT起始地址0027:0000
	struct gate_descriptor    *idt = (struct gate_descriptor    *) ADR_IDT;			//IDT起始地址0026:f800
	int i;	
	
	for (i = 0; i < 8192; i++)					//GDT初始化（0~8191个段）
		setseg(gdt + i, 0, 0, 0);					//将每个段的地址上限（终地址）、基址（始地址）、访问权限初始化为0
	setseg(gdt + 1, 0xffffffff, 0x00000000, DATA32);			//设置1号段（整个内存）
	setseg(gdt + 2, LIMIT_HARIMAIN, ADR_HARIMAIN, CODE32);			//设置2号段（整个harimain.hrb）
	io_loadgdtr(LIMIT_GDT, ADR_GDT);					//将有效设定个数和GDT内存起始地址放在CPU内的GDTR寄存器

	for (i = 0; i < 256; i++)					//IDT初始化（0~255个中断）
		setgate(idt + i, 0, 0, 0);
		
	setgate(idt+0x0c,(int)int0ch,2<<3,INTGATE32);				//设置0ch号中断
	setgate(idt+0x0d,(int)int0dh,2<<3,INTGATE32);				//设置0dh号中断
	
	setgate(idt+0x20,(int)int20h,2<<3,INTGATE32);				//设置20h号中断
	setgate(idt+0x21,(int)int21h,2<<3,INTGATE32);				//设置21h号中断
	setgate(idt+0x27,(int)int27h,2<<3,INTGATE32);				//设置27h号中断
	setgate(idt+0x2c,(int)int2ch,2<<3,INTGATE32);				//设置2ch号中断
	
	setgate(idt+0x40,(int)int40h,2<<3,INTGATE32+0x60);			//设置40h号中断为可供应用程序调用（表示该中断是可供应用程序调用的API）（应用程序只能调用+0x60的中断）（所有中断的段号属于操作系统）
	setgate(idt+0x41,(int)int41h,2<<3,INTGATE32+0x60);
	setgate(idt+0x42,(int)int42h,2<<3,INTGATE32+0x60);
	setgate(idt+0x43,(int)int43h,2<<3,INTGATE32+0x60);
	
	io_loadidtr(LIMIT_IDT, ADR_IDT);						//将有效设定个数和内存起始地址放在CPU内的IDTR寄存器
	
}

void setseg(struct segment_descriptor *seg, unsigned int limit, int base, 			int ar)		//利用参数对结构体的各变量进行设置子程序（填写GDT的8字节信息）
{	//注册段在段号记录表的基址//地址上限（终地址）//登记在该段上的内容在内存中的基址（始地址）//段属性
	if (limit > 0xfffff) 								//如果段上限超过了20位（普通情况下的段上限能指定的最大值）
	{
		ar |= 0x8000;								//设定扩展访问权高四位中的GD00中的G=1，使得该段的段上限单位解释为4kb从而使得段上限能指定最大4gb 
		limit /= 0x1000;							//此时段上限的单位为4kb故除以0x1000得到与硬件理解一致的段上限数值
	}
	seg->limit_low    = limit & 0xffff;						//取段上限的低2字节
	seg->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);	//将段上限右移2字节后使得高2字节到低2字节位置并取其中的低4位，将段属性右移1字节并取低1字节的高四位
	seg->base_low     = base & 0xffff;						//取基址的低2字节			
	seg->base_mid     = (base >> 16) & 0xff;					//将基址右移2字节后取低1字节
	seg->base_high    = (base >> 24) & 0xff;					//将基址右移3字节后取低1字节
	seg->access_right = ar & 0xff;							//取段属性的低1字节（高四位已经储存在limit_high中）
}

void setgate(struct gate_descriptor *gate, int offset,		 int selector,		 int ar)	//利用参数对结构体的各变量进行设置子程序（填写IDT的8字节信息）
{	//注册段在中断记录表的基址	//中断处理程序的入口地址//该处理程序位于哪一段//段属性
	gate->offset_low   = offset & 0xffff;
	gate->offset_high  = (offset >> 16) & 0xffff;
	gate->selector     = selector;
	gate->dw_count     = (ar >> 8) & 0xff;
	gate->access_right = ar & 0xff;	
}

void initPIC(void)						//PIC为集成了8个中断信号的可编程中断控制器，初始化PIC即初始化中断设置
{
	io_out8(PIC0_IMR,0xff);					//IMR为中断寄存器，在设置中断时先屏蔽所有中断以免在设置过程中出现中断而发生混乱
	io_out8(PIC1_IMR,0xff);					//置1表示屏蔽该位中断
	
	io_out8(PIC0_ICW1,0x11);				//ICW为初始化控制数据，ICW1与ICW4由硬件决定，必须为该值
	io_out8(PIC0_ICW2,0x20);				//设定中断号从0x20开始分别与IRQ（中断信号）0~8一一对应
	io_out8(PIC0_ICW3,1<<2);				//将1左移两位成为00000100，表示从PIC由主PIC的IRQ2连接（硬件决定，软件只能遵从）
	io_out8(PIC0_ICW4,0x01);
	
	io_out8(PIC1_ICW1,0x11);				//边沿触发模式
	io_out8(PIC1_ICW2,0x28);
	io_out8(PIC1_ICW3,2);					//表示从PIC由主PIC的IRQ2连接（直接写IRQ编号）
	io_out8(PIC1_ICW4,0x01);				//无缓冲区模式
	
	io_out8(PIC0_IMR,0xfb);					//屏蔽PIC0中除IRQ2外的所有中断（对于位的设置都是倒序后再输入）（置IRQ2为0本应是0xbf，然而输入到引脚时需要倒序）
	io_out8(PIC1_IMR,0xff);					//屏蔽PIC1所有中断
}

void initint(void)
{
	initPIC();	
	initGDTIDT();
	io_sti();
	initPIT();
	io_out8(PIC0_IMR, 0xf8); 			//开放键盘和从pic和计时器中断（键盘接IRQ1，从pic接IRQ2，计时器接IRQ0，设定PIC0的IMR为11111000）
	io_out8(PIC1_IMR, 0xef);  						//开放鼠标中断（鼠标接IRQ12，设定PIC1的IMR为11101111）	
}

void initPIT(void)						//初始化PIT以激活定时器（即设定IRQ0）
{
	io_out8(PIT_CTRL,0x34);					//根据8254芯片手册向0x43端口传送0x34
	io_out8(PIT_CNT0,0x9c);					//中断周期的低八位
	io_out8(PIT_CNT0,0x2e);					//中断周期的高八位（中断频率=cpu主频/中断周期）（即10毫秒产生一次时钟中断）
	timerctl->timerhead=0;					//初始化链表头指向0
	timerctl->counter=0;					//初始化计时器
}
