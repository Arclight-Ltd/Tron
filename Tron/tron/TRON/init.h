
			/*harimain.c�е�����*/			
void init(void);
extern char hankaku[4096];				//�������ַ����ݿ���Դ�ļ�����

struct mesg						//ȡ����ʾģʽ��Ϣ�ṹ��
{
	char cyls,leds,vmode,nop;			//�������׵�ַ���ճ�һ�ֽ�//����led��ָʾ״̬//��ɫλ������Ϣ//�ճ�һ�ֽ�
	short xnumber,ynumber;				//�ֱ�����X�ĸ���//�ֱ�����y�ĸ���(1��)
	char*vram;					//��ǰģʽ��ͼ�񻺳������׵�ַ
};							//infoΪ�ṹ��ָ�룬�õ�ַ�Ժ�������ֽ��Զ���Ϊ�ṹ�����ı�����Χ��0x0ff0-0x0ff8��ָ���ݷֱ��Ӧcyls-vram��

			/*init_int.c�е�����*/
struct segment_descriptor				//����CPU���Ͻ�GDT��8�ֽ���Ϣ���Ϊ�ṹ��
{
	short limit_low, base_low;			//baseΪ�λ�ַ���������ε���ʼ��ַ��Ϊ����X286�ʷ�Ϊ3��base��
	char base_mid, access_right;			//access_rightΪ������
	char limit_high, base_high;			//limitΪ�����ޣ��������εĴ�С
};

struct gate_descriptor 					//����CPU���Ͻ�IDT��8�ֽ���Ϣ���Ϊ�ṹ��
{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void initGDTIDT(void);
void setseg(struct segment_descriptor *seg, unsigned int limit, int base, int ar);		//����ʹ���˽ṹ�������Ҫ�ڽṹ������֮���������ú���
void setgate(struct gate_descriptor *gate, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800		//IDT��ַ
#define LIMIT_IDT		0x000007ff		//IDT���ȣ���С��
#define ADR_GDT			0x00270000		//GDT��ַ
#define LIMIT_GDT		0x0000ffff		//GDT����
#define ADR_HARIMAIN		0x00280000		//harimain.hrb�Ļ�ַ
#define LIMIT_HARIMAIN		0x0007ffff 		//harimain.hrb����ַ
#define DATA32			0x4092			//���������ݣ�ϵͳר�ã��ɶ� ��д ����ִ��
#define CODE32			0x409a			//������ָ�ϵͳר�ã��ɶ� ��ִ�� ����д
#define LDT			0x0082			//����LDT
#define INTGATE32		0x008e			//�����ж�����	
#define TSS32			0x0089			//���������л�
	
void initPIC(void);
void initint(void);
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021				//IMRΪ�жϼĴ���
#define PIC0_ICW1		0x0020				//ICWΪ��ʼ����������
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

			/*int.c�е�����*/
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

			/*intfunc.nas�е�����*/
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

			/*mouce.c�е�����*/
#define KEYBO_STATUS		0x0064
#define KEYBO_COMMAND		0x0064

struct mouse
{
	unsigned char data[3],mark,btn;						//�����ݴ�������¼�������Ƕ����ĵڼ�������,��갴��״̬
	char x,y;								//������λ��(x,y)(����Ϊ�з������������м������ʾ)
};

void waitkbc(void);
void initkeybored(void);
void enablemouse(void);
void dealkeybored(unsigned char i);
void dealmouse(unsigned char*data,unsigned char i);
 
			/*memory.c�е�����*/
unsigned int memcount(unsigned int start,unsigned int end);
#define BLOCKNUM 4090 						//�ڴ�������������Ҫ���ĵ������趨�ú궨����ں����޸ģ�
struct free
{
	void*addr;						//�����ڴ��׵�ַ
	unsigned int size;					//�ڴ���С
};
struct manage
{
	unsigned int block,freememo,lost,lostsize;		//���������ܿ����ڴ��С�������ڴ�Ĵ����������ڴ���ܴ�С
	struct free memo[BLOCKNUM];				//�ڴ����Ϣ����
};
void initmemory();
void*allocbyte(unsigned int size);
void*alloc(unsigned int size);
char freebyte(void*addr,unsigned int size);
char free(void*addr,unsigned int size);

			/*portbuf.c�е�����*/
#define KEYBOFIFO	256
struct fifo32						//��¼�������ĸ���ָ��
{
	int buf[128];					//�������׵�ַ
	int size,free,write,read;			//��������С������ռ䣬д��λ�ã�����λ��
	struct tasks*task;				//������������ʱҪ����������ַ
};	
void initbuf(struct fifo32*fifo);
int writebuf(struct fifo32*fifo,int data);
int readbuf(struct fifo32*fifo);
int bufstatus(struct fifo32*fifo);

			/*sheet.c�е�����*/
#define DESK	1
#define MOUSE	0
#define MAXSHEET 256						//���ͼ����
struct sheets							//��һͼ����Ϣ
{
	unsigned char*buf;					//��ʾͼ���ӳ������ڵ�ַ
	int xsize,ysize,x,y;					//ͼƬ�������С(1��ʼ)���������С�������ϽǺ����꣬���Ͻ������꣩
	long height,color,num;					//��Ը߶ȣ���ͼ���е���ʾλ�ã�,͸��ɫɫ��,ͼ����
	char*title;						//�����ͼ�����ڴ�Ŵ��������ã����ڼ�¼��������
	struct fifo32*fifo;
};
struct shectls							//ͼ����Ϣ�����
{
	unsigned char*map;
	long num,top;						//ͼ���ܸ���,����ʾ��ͼ�����(��Ϊ��0��ʼ����)
	struct sheets*sheet[MAXSHEET*2];			//���н�����ͼ�㣨����appͼ�㣩�ĵ�ַ
	struct sheets*sheetaddr[MAXSHEET];			//Ҫ��ʾ��ͼ���ַ
};
struct appsheets						//���ڵǼ�Ӧ�ó��򴴽���ͼ�㣬�ȶ���Ϊ���10��ͼ��(int.c����)
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

			/*display.c�е�����*/
#define ADR_MESG	0x00000ff0			//��ʼ��ʱ��¼����Ļ��ʾ��Ϣ�����ַ
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

			/*timer.c�е�����*/
#define TIMEROFF	0			
#define TIMERON		1
struct timers							//��ʱ����Ϣ
{
	unsigned char data,status;				//�ü�ʱ����ţ��ü�ʱ���Ƿ��ڹ���
	unsigned int counter,timeout;				//�ü�ʱ����counter��ʱ����
	struct fifo32*fifo;					//�ü�ʱ��ʹ�õĻ�����
	struct timers*next;					//��һ����ʱ���ĵ�ַ
};
struct apptimers						//app�ö�ʱ���ǼǱ��ݶ��Ǽ�100����ʱ��
{
	struct timers*timer[100];
	unsigned char num;
};
struct timerctls						//��ʱ�������
{
	unsigned int counter;					//��counter
	struct timers*timerhead;				//��һ��Ҫ����ļ�ʱ����ַ
};
struct timers*timeralloc(unsigned int timeout,char data,struct fifo32*fifo);
void timerfree(struct timers*timerx);
void timeron(struct timers*timerx);
void timersolder(void);

			/*console.c�е�����*/
#define		KILL	1000				//�ͷ�����������ʱ�򻺳��������ָ��			
struct conchar						//���ڼ�¼�������������Ϣ
{
	struct sheets*sheet;				//�����е�ͼ���ַ
	unsigned int cursor_x,cursor_y;			//��ǰ���λ��
	unsigned int mark_y;				//����>��ʾ�����к�
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

			/*multask.c�е�����*/
#define SLEEP	0						
#define WAKE	1						//status�����

#define	RANK	4						//����ȼ��������һ��ֻ���ڱ���		
struct tss32							//task status segment ����״̬��
{
	int backlink,esp0,ss0,esp1,ss1,esp2,ss2,cr3;		//������������ص���Ϣ���л�ʱ���ᱻд�루backlink��ʱ�ᱻд�룩
	int eip,eflags,eax,ecx,edx,ebx,esp,ebp,esi,edi;		//32λ�Ĵ���
	int es,cs,ss,ds,fs,gs;					//�μĴ���
	int ldtr,iomap;						//�����������й�,�л�ʱ���ᱻ�Զ�д��
};
struct tasks							//�����������Ϣ
{
	int sel;						//selector���ε�ַ����GDT��ţ�
	struct tss32 tss;					//�������tss��Ϣ
	char status;						//�������״̬�������������ߣ�
	unsigned char priority;					//�����񱻷���ִ�е�ʱ������λ��0.01�룩������趨Ϊ0.02~0.10�䣩
	unsigned char rank;					//����������ȼ���0�����ȣ�2���
	struct tasks*next;					//��һ������ĵ�ַ
	struct conchar conc;					//��������Ҫ�������У�����Ҫ��һЩ�ؼ�����
	struct appsheets appsheet;				//app�õ�ͼ��ͻ�����
	struct apptimers apptimer;				//app�õļ�ʱ��
	struct segment_descriptor ldt[2];			//������������õ�LDT�����ݣ�һ������app�ĳ����һ������app�����ݶΣ�
};
struct ranks
{
	int num;						//�õȼ�����ִ�е����������
	struct tasks*taskhead;					//ָ��õȼ�����ִ�е�����
};
struct taskctls							//��������
{
	int sele;						//��һ������Ķκ�
	struct tasks*taskhead;					//ָ������ִ�е���������
	struct ranks rank[RANK];				//�趨�����ȼ�����������(�����ָ�����;ͱ����ڳ����������ڴ治Ȼ��ַ������仯)
};
void initmultask(void);
struct tasks*taskalloc(void (*main)());
void inittss32(struct tss32*tss,int main);
void taskwake(struct tasks*task);
void tasksleep(struct tasks*task);
void taskrank(struct tasks*task,unsigned char rank,unsigned char priority);
void switask(void);
void soldermain(void);

			/*cmd.c�е�����*/
#define ADR_FILEINFO 	0x00102600;			//���̵�ַΪ0x00100000�������е�ַ0x002600�ĵط�����ļ�����Ϣ
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

			/*loadfile.c�е�����*/
void readfat();
void loadfile(int cluster, int size, char *buf);
