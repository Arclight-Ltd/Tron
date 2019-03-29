#include"io.h"
#include"init.h"
#include<stdio.h>
					//处理鼠标键盘时的taskhead为task_a
extern struct shectls*shectl;
extern struct mesg*info;
extern struct fifo8 keyfifo;								//定义keyfifo结构体为键盘缓冲区服务
extern struct fifo8 mousefifo;								//定义mousefifo结构体为鼠标缓冲区服务
extern unsigned char keybuf[32];							//定义keybuf数组作为键盘缓冲区储存空间
extern unsigned char mousebuf[128];							//定义mousebuf数组作为鼠标缓冲区储存空间

char cursor_x=0,shift=0x0f;								//光标位置，shift键状态
unsigned char modol=0;									//记录是否是拖动窗口模式
struct mouse variable;									//打包用于记录鼠标数据的各个变量
struct sheets*winnow;									//储存当前选中的图层

static char keytable_n[0x54]=								/*shift没按下的字符表*/
{
	0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\\',   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'' , 0,   0, '|','Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0,   0, 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};
static char keytable_s[0x54] = 								/*shift按下的字符表*/
{
	0,   0,   '!', '@', '#', '$', '%', '^', '&', '*','(', ')', '_', '+', 0,   0,  
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|',   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0,   0,   '}', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0,   0, 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};


void waitkbc(void)					//等待键盘控制电路响应子程序
{
	while(io_in8(KEYBO_STATUS)&0x02);		//从键盘状态KEYBO_STATUS端口处读取的数据的低位右数第二位为0则说明电路响应，可以接收CPU命令
}

void initkeybored(void)					//初始化（激活）键盘控制电路子程序（鼠标控制电路含在键盘控制电路中）
{
	waitkbc();
	io_out8(KEYBO_COMMAND,0x60);			//向键盘命令KEYBO_COMMAND端口输出模式设定指令
	waitkbc();
	io_out8(KEYBO_DATA,0x47);			//向键盘数据KEYBO_DATA端口输出模式号码——0x47（鼠标模式）
}

void enablemouse(void)					//激活鼠标子程序
{
	waitkbc();
	io_out8(KEYBO_COMMAND,0xd4);			//向键盘命令KEYBO_COMMAND端口输出0xd4时下一个数据会自动发送给鼠标
	waitkbc();
	io_out8(KEYBO_DATA,0xf4);			//向键盘数据KEYBO_DATA端口输出鼠标激活码0xf4，收到激活指令后会发送0xfa到cpu作为答复
}

void dealkeybored(unsigned char i)				//键盘中断处理子程序
{
	char * s=0;						//定义指针后要立马对其赋值，不然会报错，直接使用有风险
	char*keytable=0;
	char leds;
	struct tasks*task=winnow->fifo->task;			//用选中窗口图层中储存的task赋值
	leds=(info->leds>>4)&7;					//从内存中取出leds的值
	switch(i)
	{
	case 0x0e:							//如果按下了退格键
		if(winnow->fifo)
			writebuf(winnow->fifo,8);break;				//通知选中的窗口接收数据	
	case 0x3a:info->leds=(leds^=4)<<4;break;			//CapsLock为leds的第三位（修改三个锁定键）//如果有修改就先给leds赋值然后把修改后的值写回去
	case 0x45:info->leds=(leds^=2)<<4;break;			//NumLock为leds的第三位
	case 0x46:info->leds=(leds^=1)<<4;break;			//ScrollLock为leds的第三位
	case 0x2a:							//shift键
	case 0x36:shift=~shift;break;					//shift键
	case 0x1c:							//回车键
		if(winnow->fifo)
			writebuf(winnow->fifo,10);break;			//通知选中的窗口接收数据
	case 0x53:if((shift!=0x0f)&&(task->tss.ss0!=0))			//按下shift+delete时强制关闭应用程序（根据ss0检验当前是否有运行应用程序）
		{
			putstr0("		Application shut down\n");
			io_cli();					//修改寄存器时屏蔽中断
			task->tss.eax=(int)&(task->tss.esp0);	//endapp需要的参数
			task->tss.eip=(int)&endapp;		//由于处理键盘程序和命令行不是同一个任务，故不能通过调用的方式。不然将会从该任务直接跳回至命令行任务导致任务的混乱
			io_sti();				//而是直接修改命令行任务的eip。当该任务结束后跳转回命令行任务后才会调用endapp，恢复到startapp函数后的状态。
			top(task->conc.sheet);			//关闭应用程序后返回调用该程序的命令行任务并将其置顶
		}break;
	case 0x1d:initconsole();break;					//按下ctrl键打开新的命令行窗口
	default:
		keytable=(shift==0x0f)?keytable_n:keytable_s;		//如果shift键为原始状态就用keytable_n中的符号否则就用keytable_s
		if(keytable[i])
		{
			s[0]=keytable[i];				//必须通过s过渡不然会把keytable中的元素作为地址进行取地址操作从而乱码
			if(('A'<=s[0]&&s[0]<='Z')&&(!(leds&4)^(shift!=0x0f)))		//如果大写锁定键和shift同时打开且是字母才转换为小写
				s[0]|=0x20;		
			if(winnow->fifo)
				writebuf(winnow->fifo,s[0]+KEYBOFIFO);	//通知选中的窗口接收数据
		}
	}
}

void dealmouse(unsigned char*data,unsigned char i)		//鼠标中断处理子程序
{	
	int mx,my,height,x,y;							//鼠标的绝对位置，选中的图层高度，鼠标相对于选中图层的位置	
	struct tasks*task;							//用于判断关闭的是否是应用程序窗口
	struct conchar*conc=&winnow->fifo->task->conc;				//用选中窗口关联的conc为其赋值
	switch(variable.mark)							//等鼠标的三个信息都齐了再显示出来
	{
	case 1:data[0]=i;
		if((data[0]&0xc8)!=0x08)					// !=运算优先于&，故不加括号时反而会将最后一位是1的数据舍弃
			variable.mark--;					//如果第一个数据不是在规定的范围内说明数据错位，则舍弃该数据
		break;
	case 2:data[1]=i;
		break;
	case 3:	data[2]=i;
		variable.mark=0;					//恢复mark标记以准备下一次接收数据
		
		variable.btn=data[0]&0x07;				//只取数据一的第三位作为鼠标按键状态
		variable.x=data[1];					//数据二为x
		variable.y=data[2];					//数据三为y
		if(data[0]&0x10)					//根据数据一高四位对x，y作出设置
 			variable.x|=0xffffff00;
		if(data[0]&0x20)
			variable.y|=0xffffff00; 
		variable.y=-variable.y;				//鼠标与屏幕的y方向相反(向下滑动时鼠标收到的相对位移为负，但对于显示y坐标来说位移为正)			
			
/* 		if(variable.btn&0x01)					//第一位有数据说明左键按下
			s[0]='L';			//由于松开按钮时会重新执行一次sprintf语句，而此时btn为0故不会转变为大写，便产生松开变小写的效果
		if(variable.btn&0x02)					//第二位有数据说明右键按下
			s[2]='R';
		if(variable.btn&0x04)					//第三位有数据说明滚轮按下
			s[1]='C'; */
			
		mx=shectl->sheet[MOUSE]->x+variable.x;			//(mx,my)为鼠标的绝对地址
		my=shectl->sheet[MOUSE]->y+variable.y;
		if(mx<0)								//确保鼠标不出屏幕显示边界
			mx=0;
		if(mx>info->xnumber-1)
			mx=info->xnumber-1;
		if(my<0)
			my=0;
		if(my>info->ynumber-1)
			my=info->ynumber-1;
		slide(shectl->sheet[MOUSE],mx,my);					//在当前位置重新显示鼠标
		if(variable.btn&0x01)							//如果单击了左键
		{
			if(modol==0)							//如果不是移动模式
			{
				height=shectl->map[mx-1+my*info->xnumber];		//判断当前鼠标位置-1像素的位置上是哪个高度的图形
				if(height!=shectl->top)					//如果不是桌面（不可能是鼠标）
				{
					top(shectl->sheetaddr[height]);			//将该图层置顶
					x=mx-winnow->x;					//鼠标相对于图层左上角的坐标
					y=my-winnow->y;
					if(winnow->xsize-21<=x&&x<winnow->xsize-5&&5<=y&&y<19)	//如果点击了关闭按钮
					{
						task=winnow->fifo->task;			//将与窗口缓冲区绑定的task赋过去
						if((task->appsheet.num==1)||(task->appsheet.num>0&&winnow==conc->sheet))	//如果关闭的是应用程序最后一个窗口或正在运行应用程序的命令行窗口就调用退出程序功能
						{
							io_cli();				//修改寄存器时屏蔽中断
							task->tss.eax=(int)&(task->tss.esp0);	//endapp需要的参数
							task->tss.eip=(int)&endapp;		//由于处理键盘程序和命令行不是同一个任务，故不能通过调用的方式。不然将会从该任务直接跳回至命令行任务导致任务的混乱
							io_sti();				//而是直接修改命令行任务的eip。当该任务结束后跳转回命令行任务后才会调用endapp，恢复到startapp函数后的状态。
							top(task->conc.sheet);			//关闭应用程序后返回调用该程序的命令行任务并将其置顶
						}		
						else 					//如果是一般的窗口就释放该图层
						{
							if(winnow->fifo)			//如果该窗口有缓冲区
								writebuf(winnow->fifo,KILL);	//通知该窗口要被释放（由于任务优先级的限制在向fifo写入数据后不会马上切换到该任务）（为了能够释放计时器等空间，先通知该任务，由该任务进行相关处理后再通过task_a释放任务）
							else					//如果是普通窗口
							{
								free(winnow->buf,winnow->xsize*winnow->ysize);	//释放该图层的buf空间//只有buf图像缓冲区建立时用了4k内存分配法 
								shefree(winnow);				//释放该图层所占空间
							}								
						}
								
					}
					if(3<=x&&x<winnow->xsize-3&&3<=y&&y<21)			//如果鼠标落在窗口的标题栏则可以拖动
						modol=1;					//切换到窗口拖动模式						
				}
				else								//如果点击了桌面	
				{
					winnow=shectl->sheet[DESK];				//将桌面当为当前选择的图层
					if(shectl->top>1)					//如果有窗口
						wintitle(shectl->sheetaddr[1],1);		//将刚刚是置顶状态的图层标题栏置灰		
					refresh(shectl->sheetaddr[1]->x+3,shectl->sheetaddr[1]->x+shectl->sheetaddr[1]->xsize-3,shectl->sheetaddr[1]->y+3,shectl->sheetaddr[1]->y+3+18);
					if(shectl->sheetaddr[1]->fifo)				//如果刚刚的窗口有绑定缓冲区则通知其休眠
						writebuf(shectl->sheetaddr[1]->fifo,-2);
				}
			}
			else									//如果是拖动模式
				slide(winnow,winnow->x+variable.x,winnow->y+variable.y);			
		}
		else										//松开鼠标则离开拖动模式
			modol=0;
	}	
	variable.mark++;
}

