#include"io.h"
#include"init.h"

unsigned int memcount(unsigned int start,unsigned int end)		//内存总容量计算子程序（最大能检测1GB内存）
{
	char cpu486=0;						//标记是否是486及以上的cpu
	unsigned int eflag,cr0,i;				//储存标志寄存器，cr0寄存器的值，内存大小的值
					//检测是否是486及以上的cpu
	eflag=io_popeflag();					//取得当前的标志寄存器的值
	eflag|=0x40000;						//486及以上的cpu的第18位为ac，486以下的没有ac位，即使设置了ac位也无效。故将ac位设为1
	io_pusheflag(eflag);					//设置标志寄存器
	eflag=io_popeflag();					//重新读取标志寄存器
	if(eflag&0x40000)					//检测ac位。若ac为1说明为486及以上。若为0则为486以下
		cpu486=1;
	eflag&=~0x40000;					//恢复寄存器原来的值
	io_pusheflag(eflag);
					//如果是486及以上的cpu则需要屏蔽缓存才能准确地直接沟通内存
	if(cpu486)						
	{
		cr0=io_loadcr0();
		cr0|=0x60000000;
		io_storecr0(cr0);
	}
					//取得内存容量
	i=io_count(start,end);					
					//如果是486及以上的cpu则重新开启缓存
	if(cpu486)						
	{
		cr0=io_loadcr0();
		cr0&=~0x60000000;
		io_storecr0(cr0);
	}
					//返回能够使用的内存的最大的地址
	return i;
}

struct manage*memory=(struct manage*)0x003c0000;				//将内存管理表储存在0x003c0000处

void initmemory()						//初始化内存管理表子程序
{
	memory->block=0;
	memory->freememo=0;	
	memory->lost=0;	
	memory->lostsize=0;							//将各个指标设为0
	free((void*)0x00001000,0x0009e000+1);					//登记0:1000到9:f000的内存为可用（vram为a:0000到a:ffff,b:0000后为bios）
	free((void*)0x00400000,memcount(0x00400000,0xbfffffff)-0x00400000+1);	//登记0040:0000到0200:0000的内存为可用(内存大小由虚拟机分配)
}

void*allocbyte(unsigned int size)				//分配内存子程序，返回所需内存的首地址
{	//所需内存大小
	unsigned int c;						//内存区块标记
	void *addr;						//暂存首地址
	for(c=0;c<memory->block;c++)				//检测各个内存块中有无符合条件的内存容量
		if(size<=memory->memo[c].size)			//如果有
		{
			addr=memory->memo[c].addr;		//将该内存块首地址交给addr
			memory->memo[c].addr+=size;		//该内存块首地址后移
			memory->memo[c].size-=size;		//内存块的可用内存大小减少
			memory->freememo-=size;			//总可用内存减少
			if(!memory->memo[c].size)		//如果该内存块大小为0
			{
				memory->block--;		//去掉该内存块
				for(;c<memory->block;c++)	//该内存块之后的内存块前移覆盖
					memory->memo[c]=memory->memo[c+1];				
			}
			return addr;				//返回首地址
		}
	return 0;						//若内存不足则返回0
}

void*alloc(unsigned int size)				//分配4k内存子程序(以4k为单位便于内存管理)
{
	size=(size+0xfff)&0xfffff000;
	return allocbyte(size);
}

char freebyte(void*addr,		unsigned int size)	//内存释放子程序，成功返回0失败返回-1
{	//要释放内存的首地址   //要释放的大小
	int c,d;						//内存区块标记
	for(c=0;c<memory->block;c++)				//内存块已经按首地址大小从小到大排列，当要释放的首地址小于区块首地址时跳出循环
		if(addr<memory->memo[c].addr)
			break;
	if(c>0)							//保证[c-1]不越界
		if(memory->memo[c-1].addr+memory->memo[c-1].size==addr)  	//当要释放的首地址与前面一个内存块能连在一起时
		{
			memory->memo[c-1].size+=size;				//前一内存块可用内存大小增加
			memory->freememo+=size;					//总可用内存增加
			if(addr+size==memory->memo[c].addr)			//如果要释放的首地址又与后一内存块能连在一起
			{
				memory->memo[c-1].size+=memory->memo[c].size;	//将后一内存块的大小增加到前一内存块中
				memory->block--;				//内存块数量减少
				for(;c<memory->block;c++)			//内存块前移覆盖
					memory->memo[c]=memory->memo[c+1];
			}
			return 0;						
		}
	if(addr+size==memory->memo[c].addr)			//当要释放的首地址不能与前一内存块连在一起但能和后一个内存块能连在一起时
	{
		memory->memo[c].addr=addr;			//后一内存块首地址变为要释放内存的首地址
		memory->memo[c].size+=size;			//后一内存块可用内存大小增加
		memory->freememo+=size;				//总可用内存增加
		return 0;
	}			
	if(memory->block<BLOCKNUM)				//如果前后都不能连并且内存块数量在数组大小范围内
	{
		for(d=memory->block;d>c;d--)			//从后一内存块（含）开始全部内存块后移
			memory->memo[d]=memory->memo[d-1];
		memory->memo[c].addr=addr;			//将要释放的内存的信息写于“后一”内存块原本的位置
		memory->memo[c].size=size;
		memory->freememo+=size;				//总可用内存增加
		memory->block++;				//内存块数量增加
		return 0;
	}
	else							//如果内存块数已满
	{
		memory->lostsize+=size;				//将当前要释放的内存舍弃并记录入舍弃的内存大小
		memory->lost++;					//记录舍弃内存的次数
		return -1;
	}	
}

char free(void*addr,unsigned int size)			//4k内存释放子程序
{
	size=(size+0xfff)&0xfffff000;
	return freebyte(addr,size);
}
