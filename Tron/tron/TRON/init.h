
			/*harimain.c中的声明*/			
void init(void);
extern char hankaku[4096];				//声明该字符数据库在源文件以外

struct mesg						//取得显示模式信息结构体
{
	char cyls,leds,vmode,nop;			//启动区首地址，空出一字节//键盘led灯指示状态//颜色位数的信息//空出一字节
	short xnumber,ynumber;				//分辨率中X的个数//分辨率中y的个数(1起)
	char*vram;					//当前模式下图像缓冲区的首地址
};							//info为结构体指针，该地址以后的若干字节自动作为结构体打包的变量范围（0x0ff0-0x0ff8所指数据分别对应cyls-vram）

			/*init_int.c中的声明*/
struct segment_descriptor				//根据CPU资料将GDT的8字节信息打包为结构体
{
	short limit_low, base_low;			//base为段基址，即各个段的起始地址（为兼容X286故分为3个base）
	char base_mid, access_right;			//access_right为段属性
	char limit_high, base_high;			//limit为段上限，即各个段的大小
};

struct gate_descriptor 					//根据CPU资料将IDT的8字节信息打包为结构体
{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void initGDTIDT(void);
void setseg(struct segment_descriptor *seg, unsigned int limit, int base, int ar);		//由于使用了结构体变量故要在结构体声明之后再声明该函数
void setgate(struct gate_descriptor *gate, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800		//IDT基址
#define LIMIT_IDT		0x000007ff		//IDT长度（大小）
#define ADR_GDT			0x00270000		//GDT基址
#define LIMIT_GDT		0x0000ffff		//GDT长度
#define ADR_HARIMAIN		0x00280000		//harimain.hrb的基址
#define LIMIT_HARIMAIN		0x0007ffff 		//harimain.hrb的终址
#define DATA32			0x4092			//（用于数据）系统专用，可读 可写 不可执行
#define CODE32			0x409a			//（用于指令）系统专用，可读 可执行 不可写
#define LDT			0x0082			//用于LDT
#define INTGATE32		0x008e			//用于中断属性	
#define TSS32			0x0089			//用于任务切换
	
void initPIC(void);
void initint(void);
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021				//IMR为中断寄存器
#define PIC0_ICW1		0x0020				//ICW为初始化控制数据
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021

#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW1		0x00a0
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

void initPIT(void);
#define PIT_CTRL		0x0043
#define PIT_CNT0		0x0040

			/*int.c中的声明*/
#define KEYBO_DATA		0x0060
int*int_0dh(int *esp);
int*int_0ch(int *esp);			
void int_21h(int*esp);
void int_27h(int*esp);
void int_2ch(int*esp);
int*int_40h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int*int_41h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int*int_42h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int*int_43h(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);

			/*intfunc.nas中的声明*/
void int0dh(void);
void int0ch(void);
void int20h(void);
void int21h(void);
void int2ch(void);
void int27h(void);
void endapp(void);
void int40h(void);
void int41h(void);
void int42h(void);
void int43h(void);

			/*mouce.c中的声明*/
#define KEYBO_STATUS		0x0064
#define KEYBO_COMMAND		0x0064

struct mouse
{
	unsigned char data[3],mark,btn;						//数据暂存器，记录该数据是读到的第几个数据,鼠标按键状态
	char x,y;								//鼠标相对位移(x,y)(必须为有符号类型来进行计算和显示)
};

void waitkbc(void);
void initkeybored(void);
void enablemouse(void);
void dealkeybored(unsigned char i);
void dealmouse(unsigned char*data,unsigned char i);
 
			/*memory.c中的声明*/
unsigned int memcount(unsigned int start,unsigned int end);
#define BLOCKNUM 4090 						//内存区块数（可能要更改的数量设定用宏定义便于后期修改）
struct free
{
	void*addr;						//可用内存首地址
	unsigned int size;					//内存块大小
};
struct manage
{
	unsigned int block,freememo,lost,lostsize;		//区块数，总可用内存大小，丢弃内存的次数，丢弃内存的总大小
	struct free memo[BLOCKNUM];				//内存块信息数组
};
void initmemory();
void*allocbyte(unsigned int size);
void*alloc(unsigned int size);
char freebyte(void*addr,unsigned int size);
char free(void*addr,unsigned int size);

			/*portbuf.c中的声明*/
#define KEYBOFIFO	256
struct fifo32						//记录缓冲区的各项指标
{
	int buf[128];					//缓冲区首地址
	int size,free,write,read;			//缓冲区大小，空余空间，写入位置，读出位置
	struct tasks*task;				//缓冲区有数据时要激活的任务地址
};	
void initbuf(struct fifo32*fifo);
int writebuf(struct fifo32*fifo,int data);
int readbuf(struct fifo32*fifo);
int bufstatus(struct fifo32*fifo);

			/*sheet.c中的声明*/
#define DESK	1
#define MOUSE	0
#define MAXSHEET 256						//最大图层数
struct sheets							//单一图层信息
{
	unsigned char*buf;					//显示图像子程序的入口地址
	int xsize,ysize,x,y;					//图片横坐标大小(1开始)，纵坐标大小，（左上角横坐标，左上角纵坐标）
	long height,color,num;					//相对高度（在图层中的显示位置）,透明色色号,图层标号
	char*title;						//如果该图层用于存放窗口则启用，用于记录窗口名称
	struct fifo32*fifo;
};
struct shectls							//图层信息管理表
{
	unsigned char*map;
	long num,top;						//图层总个数,在显示的图层个数(均为从0开始计数)
	struct sheets*sheet[MAXSHEET*2];			//所有建立的图层（包括app图层）的地址
	struct sheets*sheetaddr[MAXSHEET];			//要显示的图层地址
};
struct appsheets						//用于登记应用程序创建的图层，先定义为最多10个图层(int.c中用)
{
	struct sheets*sheet[10];
	int num;
	struct fifo32 fifo;	
};
void initsheet();
struct sheets*shealloc(unsigned char*buf,int xsize,int ysize,long color);
void shefree(struct sheets*sheet);
void top(struct sheets*sheet);
void hide(struct sheets*sheet);
void slide(struct sheets*sheet,int x0,int y0);
void refresh(int x0,int x1,int y0,int y1);
void refreshmap(int x0,int x1,int y0,int y1,unsigned char h);

			/*display.c中的声明*/
#define ADR_MESG	0x00000ff0			//初始化时记录的屏幕显示信息储存地址
#define BACKCOL		8
void initscreen(char*p);
void initpalette(int start,int limit);
void squer(int x0,int x1,int y0,int y1,char*p,short xnumber,unsigned char color);
void displaymouse(int x,  int y,unsigned char color,char*p,short xnumber);
void printc(unsigned char a[16],int x,int y,unsigned char color,char*p,short xnumber);
void prints(char*c,int x,int y,unsigned char color,char*p,short xnumber);
void displaywords(unsigned char*s,int x0,int y0,    	char num,struct sheets*she,unsigned char fcolor,unsigned char bcolor);
void displaywindow(struct sheets*sheet, char *title,char m);
void wintitle(struct sheets*sheet,char m);

			/*timer.c中的声明*/
#define TIMEROFF	0			
#define TIMERON		1
struct timers							//计时器信息
{
	unsigned char data,status;				//该计时器标号，该计时器是否在工作
	unsigned int counter,timeout;				//该计时器的counter，时间间隔
	struct fifo32*fifo;					//该计时器使用的缓冲区
	struct timers*next;					//下一个计时器的地址
};
struct apptimers						//app用定时器登记表，暂定登记100个定时器
{
	struct timers*timer[100];
	unsigned char num;
};
struct timerctls						//计时器管理表
{
	unsigned int counter;					//总counter
	struct timers*timerhead;				//下一个要处理的计时器地址
};
struct timers*timeralloc(unsigned int timeout,char data,struct fifo32*fifo);
void timerfree(struct timers*timerx);
void timeron(struct timers*timerx);
void timersolder(void);

			/*console.c中的声明*/
#define		KILL	1000				//释放命令行任务时向缓冲区输入的指令			
struct conchar						//用于记录命令行任务的信息
{
	struct sheets*sheet;				//任务行的图层地址
	unsigned int cursor_x,cursor_y;			//当前光标位置
	unsigned int mark_y;				//最新>提示符的行号
};
void initconsole(void);
void consolemain(void);
void drawcursor(unsigned char cursor_c);
void putchar(char word);
void putstr0(char*s);
void putstr1(char*s,int num);
void newline();
void cmd(char*cmdws,unsigned char*cmdw);	
unsigned long cmdcmp(char*cmdws);

			/*multask.c中的声明*/
#define SLEEP	0						
#define WAKE	1						//status的情况

#define	RANK	4						//任务等级数（最低一级只有哨兵）		
struct tss32							//task status segment 任务状态段
{
	int backlink,esp0,ss0,esp1,ss1,esp2,ss2,cr3;		//与任务设置相关的信息。切换时不会被写入（backlink有时会被写入）
	int eip,eflags,eax,ecx,edx,ebx,esp,ebp,esi,edi;		//32位寄存器
	int es,cs,ss,ds,fs,gs;					//段寄存器
	int ldtr,iomap;						//与任务设置有关,切换时不会被自动写入
};
struct tasks							//单个任务的信息
{
	int sel;						//selector（段地址，即GDT编号）
	struct tss32 tss;					//该任务的tss信息
	char status;						//该任务的状态（工作还是休眠）
	unsigned char priority;					//该任务被分配执行的时长（单位：0.01秒）（最好设定为0.02~0.10间）
	unsigned char rank;					//该任务的优先级（0最优先，2最后）
	struct tasks*next;					//下一个任务的地址
	struct conchar conc;					//该任务（主要是命令行）所需要的一些关键数据
	struct appsheets appsheet;				//app用的图层和缓冲区
	struct apptimers apptimer;				//app用的计时器
	struct segment_descriptor ldt[2];			//储存该任务所用的LDT段数据（一个用于app的程序段一个用于app的数据段）
};
struct ranks
{
	int num;						//该等级正在执行的任务的数量
	struct tasks*taskhead;					//指向该等级正在执行的任务
};
struct taskctls							//任务管理表
{
	int sele;						//下一个任务的段号
	struct tasks*taskhead;					//指向正在执行的任务链表
	struct ranks rank[RANK];				//设定三个等级的任务链表(如果是指针类型就必须在程序中申请内存不然地址会随机变化)
};
void initmultask(void);
struct tasks*taskalloc(void (*main)());
void inittss32(struct tss32*tss,int main);
void taskwake(struct tasks*task);
void tasksleep(struct tasks*task);
void taskrank(struct tasks*task,unsigned char rank,unsigned char priority);
void switask(void);
void soldermain(void);

			/*cmd.c中的声明*/
#define ADR_FILEINFO 	0x00102600;			//磁盘地址为0x00100000，磁盘中地址0x002600的地方存放文件名信息
void error(char*cmdws);
void name(char*cmdws);
void mem(char*cmdws);
void cls(char*cmdws);
void dir(char*cmdws);
void run(char*cmdws);
void opencmd(char*cmdws);
void exit(char*cmdws);	
void whatareyou(char*cmdws);
void father(char*cmdws);

			/*loadfile.c中的声明*/
void readfat();
void loadfile(int cluster, int size, char *buf);
