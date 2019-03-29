#include"init.h"
#include"io.h"
#include<stdio.h>
#include<string.h>

extern struct shectls*shectl;	
extern struct manage* memory;
extern struct appsheets appsheet;
extern struct sheets*winnow;						//当前选中的窗口

struct fileinfos
{
	unsigned char name[8], ext[3], type;				//8字节文件名（不足补空格），扩展名，文件属性（P367）
	char reserve[10];						//保留空间
	unsigned short time, date, cluster;				//文件生成时间，日期，文件存放扇区（簇）
	unsigned int size;						//文件大小
};
struct fileinfos*fileinfo=(struct fileinfos*)ADR_FILEINFO;

/**************************************************************************************************************************************/

void error(char*cmdws)									//错误指令执行的子程序
{
	putstr0("Sorry,I don't understand\n\n");
}

void name(char*cmdws)									//询问系统姓名
{
	putstr0("My name is Tron\n\n");
}

void mem(char*cmdws)									//询问当前内存使用状况
{
	char*s=0;
	sprintf(s,"I have %dMB memery in total\n",memcount(0x00400000,0xffffffff)/(1024*1024));
	putstr0(s);
	sprintf(s,"But now I have only %dKB in free\n\n",memory->freememo/1024);
	putstr0(s);
}

void cls(char*cmdws)									//清屏（命令行窗口）
{
	struct conchar*conc=&winnow->fifo->task->conc;
	squer(4,256*2-3,24,165*2-5,shectl->sheet[3]->buf,shectl->sheet[3]->xsize,0);
	refresh(4+shectl->sheet[3]->x,256*2-3+shectl->sheet[3]->x,24+shectl->sheet[3]->y,165*2-5+shectl->sheet[3]->y);
	conc->cursor_x=1;
	conc->cursor_y=0;
}

void dir(char*cmdws)									//显示系统文件信息
{
	char*s=0;
	unsigned char x,y;
	putstr0("  filename            size\n");
	for(x=0;x<224;x++)
	{
		if(fileinfo[x].name[0]==0)				//文件名第一个字节为0说明这一段不包含任何文件名信息
			break;
		if(fileinfo[x].name[0]!=0xe5)				//文件名第一个字节为0xe5说明该文件被删除
			if((fileinfo[x].type&0x18)==0)		//排除目录和非文件信息的段
			{
				sprintf(s,"filename.ext       %7dB\n",fileinfo[x].size);	//录入文件大小
				for(y=0;y<8;y++)			//录入文件名
					s[y]=fileinfo[x].name[y];
				s[9]=fileinfo[x].ext[0];		//录入扩展名
				s[10]=fileinfo[x].ext[1];
				s[11]=fileinfo[x].ext[2];
				putstr0(s);
			}
	}
	putchar('\n');						//""会用该字符串首地址赋值，‘’则是直接将该字符的ascll码作为实参
}

void run(char*cmdws)									//运行应用程序（run+空格+文件名）
{
	char*file,*filestack,*po;
	char name[11];
	unsigned char x;
	int c;
	int segsize,datasize,esp,datahrb;
	struct tasks*task;
	for(x=0;x<8;x++)						//用空格填充name
		name[x]=0x20;
	po=strstr(cmdws,"RUN");						//找到run在字符串中的位置
	for(x=4;(cmdws[x]!=0)&&(x<8+4)&&(po[x]!='.');x++)		//只将run后的文件名赋给name
		name[x-4]=po[x];
	name[8]='H';
	name[9]='R';
	name[10]='B';							//默认文件后缀以免执行其他文件
	for(x=0;x<224;x++)						//查找文件
	{
		if(fileinfo[x].name[0]==0)				//文件名第一个字节为0说明这一段不包含任何文件名信息
			break;
		if(fileinfo[x].name[0]!=0xe5)				//文件名第一个字节为0xe5说明该文件被删除
			if((fileinfo[x].type&0x18)==0)			//排除目录和非文件信息的段
				if(!memcmp(fileinfo[x].name,name,11))	//如果name中读取的文件和磁盘中的文件名完全相符（包括填充用的空格）
					break;				//跳出for
	}
	if((x<224)&&(fileinfo[x].name[0]!=0))				//如果不是因为x越界或name[0]=0导致的跳出（说明找到了可以执行的文件）
	{
		file=alloc(fileinfo[x].size);				//在内存中开辟一块临时的空间存放该应用程序
		task=winnow->fifo->task;		
		loadfile(fileinfo[x].cluster,fileinfo[x].size,file);	//将该程序完整地从磁盘中读到内存
											
		if(fileinfo[x].size>=36&&memcmp(file+4,"Hari",4)==0&&file[0]==0)//如果该执行文件是通过bim2hrb生成的,第一个字节一定为0，第四个字节开始一定为Hari
		{
			segsize=*((int*)(file+0x00));				//应用程序所需数据段大小(必须把file转化为int*型才能一次读取4个字节，不然只是把一个字节赋值过去将出错)
			esp=*((int*)(file+0x0c));				//ESP初始值（往前为栈往后为内存数据段）
			datasize=*((int*)(file+0x10));				//应用程序数据部分大小
			datahrb=*((int*)(file+0x14));				//应用程序数据部分首地址
			filestack=alloc(segsize);				//为应用程序开辟新的数据空间
			*((int*)0xfe8)=(int)filestack;				//将应用程序的数据段在内存中的临时首地址储存在0xfe8中															
			for(c=0;c<datasize;c++)
				filestack[esp+c]=file[datahrb+c];		//将应用程序的数据部分复制到应用程序的数据段
			setseg(task->ldt+0, fileinfo[x].size-1,(int)file, CODE32+0x60);	//设置该应用程序的指令段（将访问权限+0x60可将段设为应用程序用，这样设置后当cs在应用程序与操作系统间切换时ss/ds也会自动切换（类似于tss的工作机制）（将应用程序登记在LDT中而不是GDT中，这样，当其他应用程序在运行的时候就不能互相访问，保护应用程序）
			setseg(task->ldt+1, segsize-1,(int)filestack, DATA32+0x60);	//设置该应用程序的数据段（cs指向应用程序段时若存入操作系统段将会引发一般保护异常，访问权限+0x60的段只能访问权限同样加了0x60的段）		
											//在应用程序模式中，in、out、cli、sti、hlt指令和对系统段的操作、对系统程序的call、jmp（通过API调用除外）均会被认定为一般保护异常
			io_startapp(0x1b,0*8+4,esp,1*8+4,&(task->tss.esp0));	//就跳转到ip为0x1b开始执行.设置各寄存器后调用该应用程序（+4表示这是LDT中的段号，会自动从GDT中找出相应LDT的地址）（由于每个任务的LDT都是互相独立的（就像局部变量）所以就算段号相同也没事）（app在LDT中段号相同，各LDT在GDT中登记时的段号不同）
			if(task->appsheet.num)					//应用程序结束后检查有无遗留窗口，有的话逐一释放
			{
				for(c=0;c<task->appsheet.num;c++)
					shefree(task->appsheet.sheet[c]);
			}
			if(task->apptimer.num)					//应用程序结束后检查有无遗留计时器，有的话逐一释放
			{
				for(c=0;c<task->apptimer.num;c++)
					timerfree(task->apptimer.timer[c]);
			}
			free(filestack,segsize);
			putchar('\n');
		}	
		else								//其他文件则从头开始执行
			putstr0("It's not a .hrb file \n\n");		
		free(file,fileinfo[x].size);					//释放临时空间
	}
	else									//如果没有该文件
		putstr0("File not found\n\n");
}

void opencmd(char*cmdws)							//打开新的命令行窗口		
{
	putstr0("new console opened\n\n");
	initconsole();
}

void exit(char*cmdws)								//关闭命令行窗口并释放与该任务有关的所有空间	
{
	writebuf(winnow->fifo,KILL);						//通知自己需要被释放
}

void whatareyou(char*cmdws)	
{
	putstr0("My name is Tron,my father is Lou Cheng,and my mother is still in searching.\n");
	putstr0("My birthday is April 15th,2016.I'm glad to be bron.\n");
	putstr0("As a newly-bron system,I have limited functions yet.\n");
	putstr0("But you can ask me about memery conditon,files I include,run  apps,open and close consoles.\n");
	putstr0("You don't need to lean the commands,just ask in the way you   like.\n\n");
}

void father(char*cmdws)
{
	putstr0("My father is the most handsome man in the world.\nthe most handsome,the most handsome,the most handsome.\n\n");
}
