#include"io.h"
#include"init.h"

extern struct shectls*shectl;
struct mesg*info=(struct mesg *)ADR_MESG;
	
static unsigned char rgb[16*3]=			//设置调色板数据库
{						/*设为静态变量，表示直接定义数据而不是一个一个赋值以节省程序字节数*/
	0x00, 0x00, 0x00,			/*  0:黒 		*/
	0xff, 0x00, 0x00,			/*  1:亮红		*/		/*数组中的注释要用*不然数据会出错 */
	0x00, 0xff, 0x00,			/*  2:亮緑 		*/
	0xff, 0xff, 0x00,			/*  3:亮黄		*/
	0x00, 0x00, 0xff,			/*  4:亮蓝		*/
	0xff, 0x00, 0xff,			/*  5:亮紫 		*/
	0x00, 0xff, 0xff,			/*  6:浅亮蓝		*/
	0xff, 0xff, 0xff,			/*  7:白 		*/
	0xc6, 0xc6, 0xc6,			/*  8:亮灰 		*/
	0x84, 0x00, 0x00,			/*  9:暗红 		*/
	0x00, 0x84, 0x00,			/*  10:暗绿 		*/
	0x84, 0x84, 0x00,			/*  11:暗黄 		*/
	0x00, 0x00, 0x84,			/*  12:暗蓝 		*/
	0x84, 0x00, 0x84,			/*  13:暗紫 		*/
	0x00, 0x84, 0x84,			/*  14:浅暗蓝 		*/
	0x84, 0x84, 0x84			/*  15:暗灰 		*/
};
 
void initscreen(char*p)				//初始化屏幕显示子程序
{	
	squer(0,info->xnumber,0,info->ynumber,p,info->xnumber,BACKCOL);	//向VRAM显存写入像素颜色	//背景		//各参数由assembly.nas开头储存的模式信息中取得
	squer(0,80,info->ynumber-40,info->ynumber,p,info->xnumber,4);      //UI		//以使系统在其他模式下也能正常运行
	squer(info->xnumber-80,info->xnumber,info->ynumber-40,info->ynumber,p,info->xnumber,4);		//UI
	squer(1,80-1,info->ynumber-40+1,info->ynumber-1,p,info->xnumber,BACKCOL);		//UI
	squer(info->xnumber-80+1,info->xnumber-1,info->ynumber-40+1,info->ynumber-1,p,info->xnumber,BACKCOL);		//UI
}

void initpalette(int start,int end)		//设置调色板色号子程序
{		//要设置的调色板起始标号
	int eflag,i=0;				//i是rgb元素标号
	unsigned char c;			//色号c最大为255
	unsigned char table2[216 * 3];		//色阶法
	int r, g, b;	
	eflag=io_popeflag();			//储存此时的标志寄存器状态
	io_cli();				//关闭中断标志位
	io_out8(0x3c8,start);			//向0x3c8输入要设定的调色板编号（后面省略该步表示设定下一个编号，但第一个不能省略）
	for(c=start;c<=end;c++)		//向0x3c9分别输入RGB来设定该编号的颜色状态
	{
		io_out8(0x3c9,rgb[i]/4);	//向0x3c9端口输出R
		io_out8(0x3c9,rgb[i+1]/4);	//向0x3c9端口输出G
		io_out8(0x3c9,rgb[i+2]/4);	//向0x3c9端口输出B
		i+=3;
	} 
	
	for (b = 0; b < 6; b++) 		//以下为色阶法调色 
		for (g = 0; g < 6; g++) 
			for (r = 0; r < 6; r++) 
			{
				table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
				table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
	for(c=16;c<=231;c++)		//向0x3c9分别输入RGB来设定该编号的颜色状态
	{
		io_out8(0x3c9,table2[i]/4);	//向0x3c9端口输出R
		io_out8(0x3c9,table2[i+1]/4);	//向0x3c9端口输出G
		io_out8(0x3c9,table2[i+2]/4);	//向0x3c9端口输出B
		i+=3;
	} 
	io_pusheflag(eflag);			//恢复标志寄存器状态
}

void squer(int x0,int x1,int y0,int y1,  		 char*p,   short xnumber,unsigned char color)		//显示矩形子程序	
{	//横坐标始末地址//y坐标始末地址（相对于写入图像）//要写入图像的首地址//横坐标像素数//矩形颜色						
	int x,y;					
	for(x=x0;x<x1;x++)				//*P必须设为指向char型才能在加1是指向下一个字节地址（下一个像素），若为int则是隔1个像素再显示
		for(y=y0;y<y1;y++)			//*((int*)0x0ff8)赋给p所以要用两个字节，char*p用于p[x+y*xnumber]=color故要设为一个字节一个字节地加
			p[x+y*xnumber]=color;		//*(p+1)里的+1是加1字节还是加1字取决于p指向的类型（即此处的char*p）（p已经在被赋值时就被解释为字类型）
}

void displaymouse(int x,  int y,unsigned char color,char*p,short xnumber)				//显示鼠标形状子程序
{	//横坐标起始地址//y坐标起始地址//鼠标的透明色//显存首地址//载体横坐标像素数
	static unsigned char cursor[16][16] = 			/*设置鼠标形状的数据库*/
	{
		"**..............",
		"*O*.............",
		"*OO*............",
		"*OOO*...........",
		"*OOOO*..........",
		"*OOOOO*.........",
		"*OOOOOO*........",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOOO*****.....",
		"*OO*OO*.........",
		"*O*.*OO*........",
		"**..*OO*........",
		"*....*OO*.......",
		".....*OO*.......",
		"......**........"
	};
	unsigned char a,b;						//数组下标不能为负数
	p+=x+y*xnumber;	
	for(a=0;a<16;a++)						//0-15共16位（使用for和数组时要特别注意），若为<=16则会溢出导致显示问题
		for(b=0;b<16;b++)
			switch(cursor[a][b])
			{
			case'*':p[a*xnumber+b]=0;break;			//在*的字节显示黑色（边框）
			case'O':p[a*xnumber+b]=7;break;			//在0的字节显示要求的颜色
			default:p[a*xnumber+b]=color;			//其他地区不更改颜色，即用背景色
			}
}

void printc(unsigned char a[16],  int x,   int y,    unsigned char color,  char*p,    short xnumber)	//显示单个字符子程序
{	//要显示字符的数据库//开始显示字符方格的坐标（x，y）//字符颜色//vram显存首地址//横坐标像素数					
	unsigned int i,o;
	int byte;
	p+=x+y*xnumber;					//p=开始输入数据的首地址
	for(i=0;i<16;i++)				//共16个数据要输入（纵坐标有16位）（16*8长方形像素点阵显示字符）
	{
		byte=(int)a[i];				//从数据库取得字符
		for(o=0;o<8;o++)			//每个数据有8位(横坐标有8位)
		{
			if(byte&0x80)			//看byte的第八位是否为1，若为1则该改变颜色
				p[i*xnumber+o]=color;	//i相当于纵坐标变化，o相当于横坐标变化
			byte=io_shl(byte);		//将byte左移一位，继续下一次判断（将第七位移至第八位）
		}		
	}	
}

void prints(char*c,  		int x,   int y,   unsigned char color,char*p,short xnumber)				//显示字符串子程序
{	//字符串//开始显示字符串的左上角坐标（x，y）//字符颜色//字符写入的图像首地址//该图像横坐标像素数
	for(;*c!=0;c++,x+=8)						//一直显示直到字符串结束标志‘\0’，每显示一个字符串地址加一指向下一个字符，横坐标加8在同一行显示下个字符
		printc(hankaku+*c*16,x,y,color,p,xnumber);
}

void displaywords(unsigned char*s,int x0,int y0,    	char num,struct sheets*she,unsigned char fcolor,unsigned char bcolor)
{		//字符串//在要写入图层中的位置的左上角坐标//字符个数//字符写入的图层地址	//字符颜色	//字符背景色
	squer(x0,x0+num*8,y0,y0+16,she->buf,she->xsize,bcolor);
	prints(s,x0,y0,fcolor,she->buf,she->xsize);
	refresh(x0+she->x,x0+num*8+she->x,y0+she->y,y0+16+she->y);				//根据相对位置计算出在vram中的绝对位置后进行刷新
}				//坐标包含起点但不含终点

void wintitle(struct sheets*sheet,	char m)
{		//窗口所在图层地址//标题栏是黑（0）还是深灰（非0）
	static char closebtn[14][16] = 
	{
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c,tc=0;
	if(m)
		tc=15;
	squer(3,sheet->xsize-3,3,3+18,sheet->buf,sheet->xsize,tc); 				//标题栏黑色
	prints(sheet->title,4,4,7,sheet->buf,sheet->xsize);					//标题栏文字
	for (y = 0; y < 14; y++) 					//关闭按钮图案
		for (x = 0; x < 16; x++) 
		{
			c=closebtn[y][x];
			switch (c)
			{
			case'@':c =0;break;
			case'$':c =15;break;
			case'Q':c =8;break;
			default:c =7;
			}					
			sheet->buf[(5 + y) * sheet->xsize + (sheet->xsize - 21 + x)] = c;
		}
}

void displaywindow(struct sheets*sheet, char *title,char m)
{		//窗口所在图层地址//标题文字//标题栏是黑（0）还是深灰（非0）
	squer(0,sheet->xsize-1,0,1,sheet->buf,sheet->xsize,7);						//白色上边框
	squer(0,1,1,sheet->ysize-1,sheet->buf,sheet->xsize,7);						//白色左边框
	squer(0,sheet->xsize,sheet->ysize-1,sheet->ysize,sheet->buf,sheet->xsize,0);			//黑色右边框
	squer(sheet->xsize-1,sheet->xsize,sheet->ysize,sheet->ysize-1,sheet->buf,sheet->xsize,0);	//黑色下边框  （制造阴影效果）
	
	squer(1,sheet->xsize-1,1,sheet->ysize-1,sheet->buf,sheet->xsize,15);				//底板暗灰    （制造立体效果）
	squer(2,sheet->xsize-2,2,sheet->ysize-2,sheet->buf,sheet->xsize,8);				//窗体内容亮灰
	
	sheet->title=title;
	wintitle(sheet,m);
}
