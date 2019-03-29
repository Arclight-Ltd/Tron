#include"init.h"
#define LEN sizeof (struct sheets)				//单一图层信息结构体的大小

extern struct mesg*info;
extern struct sheets*winnow;

struct shectls*shectl=(struct shectls*)0x003d0000;			//设定图层管理表地址

void initsheet()							//初始化图层设置子程序
{
	unsigned char*mousebuf,*backgroundbuf,*windowbuf;
	struct sheets*mouse,*background,*window;				//声明部分不会分配内存，只有在使用时才会分配内存（？）
	shectl->num=-1;
	shectl->top=0;							//图层数均初始化为-1
	shectl->map=alloc(info->xnumber*info->ynumber);
	
	mousebuf=alloc(16*16);						//获取鼠标图像数组
	displaymouse(0,0,99,mousebuf,16);				//定义透明色为99
	mouse=shealloc(mousebuf,16,16,99);				//获取鼠标图层（0号）		
	mouse->height=0;						//设置鼠标显示高度
	shectl->sheetaddr[0]=mouse;					//将鼠标图层登记在显示图层数组中
	slide(mouse,info->xnumber/2,info->ynumber/2);			//设置鼠标初始化坐标
	
	backgroundbuf=alloc(info->xnumber*info->ynumber);		//获取桌面图像数组
	initscreen(backgroundbuf);					//向桌面图像写入初始化图形
	background=shealloc(backgroundbuf,info->xnumber,info->ynumber,-1);	//获取桌面背景图层（1号）
	
	windowbuf=alloc(160*68);					//获取窗口数组
	window=shealloc(windowbuf,160,68,-1);				//获取窗口图层（2号）
	displaywindow(window,"timer",0);				//向窗口图像写入初始化图形
	slide(window,80,72);						//设置窗口初始化坐标
	
	refreshmap(0,info->xnumber,0,info->ynumber,shectl->top);
	top(background);						//置顶桌面
	top(window);
}

struct sheets*shealloc(unsigned char*buf,int xsize,int ysize,long color)	//新建图层子程序，返回图层地址
{		//图片地址	//图片横坐标大小//纵坐标大小//透明色色号
	shectl->num++;									//总图层数加一
	if(shectl->num<MAXSHEET*2)
	{
		shectl->sheet[shectl->num]=allocbyte(LEN);				//每新建一个图层就向内存申请(由于申请的空间较小故用allocbyte)
		shectl->sheet[shectl->num]->buf=buf;					//根据形参设定图层信息
		shectl->sheet[shectl->num]->color=color;
		shectl->sheet[shectl->num]->x=0;					//设置初始化坐标
		shectl->sheet[shectl->num]->y=0;
		shectl->sheet[shectl->num]->xsize=xsize;
		shectl->sheet[shectl->num]->ysize=ysize;
		shectl->sheet[shectl->num]->height=-1;					//设置高度为-1（隐藏）
		shectl->sheet[shectl->num]->num=shectl->num;				//记录该图层是第几号图层
		shectl->sheet[shectl->num]->fifo=0;
		return shectl->sheet[shectl->num];					//返回图层地址
	}
	return 0;									//如果图层已满则分配失败，返回0
}

void shefree(struct sheets*sheet)			//释放图层子程序
{		//图层地址
	long c;
	hide(sheet);								//先将该图层隐藏
	for(c=0;c<sheet->fifo->task->appsheet.num;c++)				//如果该图层是应用程序窗口
		if(sheet==sheet->fifo->task->appsheet.sheet[c])			//在该命令行使用的应用程序图层管理表中将其删除（覆盖）
		{
			for(;c<sheet->fifo->task->appsheet.num;c++)
				sheet->fifo->task->appsheet.sheet[c]=sheet->fifo->task->appsheet.sheet[c+1];
			sheet->fifo->task->appsheet.num--;			//该命令行运行的app数量减一
			break;
		}					
	for(c=sheet->num;c<shectl->num;c++)			//一般图层释放：将该图层以后的所有图层前移（将该图层信息从表格中移除）
		shectl->sheet[c]=shectl->sheet[c+1];		//(根据序号抹去图层虽然方便但是可能会被黑客利用删去有用图层)（若为安全则应利用检索法）
	shectl->num--;									//总图层数减一
	freebyte(sheet,LEN);								//在内存中释放该空间
}

void top(struct sheets*sheet)							//将图层置顶（鼠标图层除外）子程序		
{	//图层地址
	long c;
	if(sheet->height>-1)								//如果当前图层已经在显示了
	{	
		for(c=sheet->height;c>1;c--)						//图层高度即该图层在正在显示的图层数组中的标号
		{
			shectl->sheetaddr[c]=shectl->sheetaddr[c-1];			//将该图层前的所有图层（鼠标外）后移
			shectl->sheetaddr[c]->height++;					//将移动的图层高度加一
		}
		shectl->sheetaddr[1]=sheet;						//将当前图层地址设置到显示图层数组的第一个
		sheet->height=1;							//该图层的高度设为1
		if(shectl->top>2)							//如果显示的图层数量大于2说明有至少两个窗口
		{										//避免将桌面图层涂上窗口标题栏
			wintitle(shectl->sheetaddr[2],1);				//将刚刚被下调的窗口的标题栏涂灰
			if(shectl->sheetaddr[2]->fifo)
				writebuf(shectl->sheetaddr[2]->fifo,-2);		//通知被下调的窗口
		}
		if(shectl->top>1)							//如果显示的图层大于1说明至少有一个图层
		{
			wintitle(sheet,0);						//将置顶的窗口的标题栏涂黑
			if(shectl->sheetaddr[1]->fifo)
				writebuf(shectl->sheetaddr[1]->fifo,-1);		//通知被置顶的窗口
		}	
	}
	else
	{
		sheet->height=1;							//将该图层高度设为1
		shectl->top++;								//在显示的图层数加一
		if(shectl->top>=MAXSHEET)						//若显示的图层满了则舍弃最底部的图层
			shectl->top=MAXSHEET-1;
		if(shectl->top>1)
			for(c=shectl->top;c>1;c--)						//将所有显示的图层（除鼠标外）后移
			{
				shectl->sheetaddr[c]=shectl->sheetaddr[c-1];
				shectl->sheetaddr[c]->height++;					//显示图层的移动都要伴随着图层高度的改变
			}			
		shectl->sheetaddr[1]=sheet;						//将该图层地址登记在置顶（除鼠标）的图层位置
		if(shectl->top>2)							//如果显示的图层数量大于2说明有至少两个窗口
		{										//避免将桌面图层涂上窗口标题栏
			wintitle(shectl->sheetaddr[2],1);				//将刚刚被下调的窗口的标题栏涂灰
			if(shectl->sheetaddr[2]->fifo)
				writebuf(shectl->sheetaddr[2]->fifo,-2);		//通知被下调的窗口
		}
		if(shectl->top>1)							//如果显示的图层大于1说明至少有一个图层
		{
			wintitle(sheet,0);						//将置顶的窗口的标题栏涂黑
			if(shectl->sheetaddr[1]->fifo)
				writebuf(shectl->sheetaddr[1]->fifo,-1);		//通知被置顶的窗口
		}
	}	
	winnow=sheet;									//储存置顶图层的地址（保持winnow指向置顶图层）
	refreshmap(0,info->xnumber,0,info->ynumber,shectl->top);			//图层高度发生变化时就要所有map标记点都刷新
	refresh(shectl->sheetaddr[2]->x+3,shectl->sheetaddr[2]->x+shectl->sheetaddr[2]->xsize-3,shectl->sheetaddr[2]->y+3,shectl->sheetaddr[2]->y+3+18);
	refresh(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize);		//只刷新显示置顶图层
}

void hide(struct sheets*sheet)							//将图层隐藏子程序
{	//图层地址
	long c;
	if(sheet->height>-1)								//如果该层在显示
	{		
		for(c=sheet->height;c<shectl->top;c++)					//将该层从显示图层数组中除去
		{
			shectl->sheetaddr[c]=shectl->sheetaddr[c+1];			//该图层以后的所有图层前移
			shectl->sheetaddr[c]->height--;					//移动后图层高度减一
		}			
		shectl->top--;								//显示的图层数减一
		sheet->height=-1;							//该图层高度设为-1
	}
	refreshmap(0,info->xnumber,0,info->ynumber,shectl->top);			//图层高度发生变化时就要所有map标记点都刷新
	refresh(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize);		//所有图层重新刷新
	top(shectl->sheetaddr[1]);							//将隐藏后的位于显示图层数组的首位图层置顶（实现关闭任务时自动切换到上一个置顶过的窗口）
}

void slide(struct sheets*sheet,int x0,int y0)					//移动图层子程序
{	//图层地址	   //目的坐标
	int oldx,oldy;
	oldx=sheet->x;
	oldy=sheet->y;
	sheet->x=x0;								//根据位移计算绝对地址
	sheet->y=y0;
	if(sheet->height>-1)							//如果该图层正在显示则刷新
	{
		refreshmap(oldx,oldx+sheet->xsize,oldy,oldy+sheet->ysize,shectl->top);	    //只刷新平移的图层所在的map标记点（原位所有图层都要更新）
		refreshmap(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize,1);	//移动后的位置只需刷新置顶图层和鼠标图层即可
		refresh(oldx,oldx+sheet->xsize,oldy,oldy+sheet->ysize);		//只刷新平移的地方（同上）
		refresh(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize);
	}
}

void refresh(int x0,int x1,			int y0,int y1)				//刷新显示图层子程序
{	//要刷新的区域在vram中的始末横坐标//及纵坐标
	struct sheets*she;							//暂存要显示的图层
	long c,color;
	int x,y;
	int dx0,dx1,dy0,dy1;
	if(x0<0)							//尽量避免在循环中出现较多工作
		x0=0;						//当左上角坐标超出边框时进行修正，刷新范围不超出屏幕
	if(y0<0)
		y0=0;
	if(x1>info->xnumber)							//尽量避免在循环中出现较多工作
		x1=info->xnumber;						//当右下角坐标超出边框时进行修正，刷新范围不超出屏幕
	if(y1>info->ynumber)
		y1=info->ynumber;
	for(c=shectl->top;c>-1;c--)
	{
		she=shectl->sheetaddr[c];						//获取要显示的图层中的图像的地址				
		dx0=x0-she->x;							//根据其在桌面上的绝对坐标计算其相对该图层首地址的坐标
		dy0=y0-she->y;							//即由vram上的绝对坐标转化为在图层图像数组中的相对坐标
		dx1=x1-she->x;
		dy1=y1-she->y;
		if(dx0<0)
			dx0=0;
		if(dy0<0)
			dy0=0;
		if(dx1>she->xsize)
			dx1=she->xsize;
		if(dy1>she->ysize)					//利用该图层边界调整显示范围
			dy1=she->ysize;
									//如果没有重合部分，那么x<dx1将不成立会直接跳过不执行
										//故可以省略判断有无重合部分的语句
		for(x=dx0;x<dx1;x++)					//显示该图层中要刷新的部分（x,y为在该图层图像数组中的位置）		
			for(y=dy0;y<dy1;y++)			
			{
				if(shectl->map[x+she->x+(y+she->y)*info->xnumber]!=she->height)		//如果是该图层的位置就写入像素
					continue;
				color=she->buf[x+y*she->xsize];			//取像素的方向要与写像素的方向一致（此处均为纵向）
				info->vram[x+she->x+(y+she->y)*info->xnumber]=color;	
			}
				
																
	}	
}

void refreshmap(int x0,int x1,			int y0,int y1,unsigned char h)		//标记图层位置子程序
{		//要刷新的区域在vram中的始末横坐标//及纵坐标//显示的最底层的图层高度
	struct sheets*she;							//暂存要显示的图层
	long c,color;
	int x,y;
	int dx0,dx1,dy0,dy1;
	if(x1>info->xnumber)							//尽量避免在循环中出现较多工作
		x1=info->xnumber;						//当右下角坐标超出边框时进行修正，刷新范围不超出屏幕
	if(y1>info->ynumber)
		y1=info->ynumber;
	for(c=h;c>-1;c--)
	{
		she=shectl->sheetaddr[c];						//获取要显示的图层中的图像的地址				
		dx0=x0-she->x;							//根据其在桌面上的绝对坐标计算其相对该图层首地址的坐标
		dy0=y0-she->y;							//即由vram上的绝对坐标转化为在图层图像数组中的相对坐标
		dx1=x1-she->x;
		dy1=y1-she->y;
		if(dx0<0)
			dx0=0;
		if(dy0<0)
			dy0=0;
		if(dx1>she->xsize)
			dx1=she->xsize;
		if(dy1>she->ysize)					//利用该图层边界调整显示范围
			dy1=she->ysize;
									//如果没有重合部分，那么x<dx1将不成立会直接跳过不执行
		if(she->color==-1)					//如果没有透明色
			for(x=dx0;x<dx1;x++)					//显示该图层中要刷新的部分（x,y为在该图层图像数组中的位置）		
				for(y=dy0;y<dy1;y++)	
				{
					color=she->buf[x+y*she->xsize];				
					shectl->map[x+she->x+(y+she->y)*info->xnumber]=c;	
				}	
		else
			for(x=dx0;x<dx1;x++)					//显示该图层中要刷新的部分（x,y为在该图层图像数组中的位置）		
				for(y=dy0;y<dy1;y++)	
				{
					color=she->buf[x+y*she->xsize];	
					if(color!=she->color)			//如果不是透明色则将该图层高度写入map				
						shectl->map[x+she->x+(y+she->y)*info->xnumber]=c;	
				}														

	}
}
