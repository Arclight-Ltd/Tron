
void io_hlt(void);							//待机
void io_cli(void);							//关中断
void io_sti(void);							//开中断
void io_stihlt(void);							//打开中断后待机
char io_in8(int port);							//从端口读取8位数据
short io_in16(int port);						//从端口读取16位数据
int io_in32(int port);							//从端口读取32位数据
void io_out8(int port,int data);					//向端口输出8位数据
void io_out16(int port,int data);					//向端口输出16位数据
void io_out32(int port,int data);					//向端口输出32位数据
int  io_popeflag(void);							//读取此时标志寄存器状态
void io_pusheflag(int eflag);						//修改标志寄存器数据
int  io_shl(int byte);							//按位向左移位一位
void io_loadgdtr(int limit, int addr);					//向GDTR寄存器写数值
void io_loadidtr(int limit, int addr);					//向IDTR寄存器写数值
int io_loadcr0(void);							//读取CR0数据
void io_storecr0(int cr0);						//修改CR0数据
unsigned int io_count(unsigned int start,unsigned int end);		//计算内存大小
void io_loadtr(int tr);							//修改TR寄存器
void io_jmpfar(int eip,int cs);						//跳转到cs:eip
void io_callfar(int eip,int cs);					//调用cs:eip指向的子程序
void io_startapp(int eip, int cs, int esp, int ds, int *tss_esp0);
