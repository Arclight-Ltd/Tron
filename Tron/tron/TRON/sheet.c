#include"init.h"
#define LEN sizeof (struct sheets)				//��һͼ����Ϣ�ṹ��Ĵ�С

extern struct mesg*info;
extern struct sheets*winnow;

struct shectls*shectl=(struct shectls*)0x003d0000;			//�趨ͼ�������ַ

void initsheet()							//��ʼ��ͼ�������ӳ���
{
	unsigned char*mousebuf,*backgroundbuf,*windowbuf;
	struct sheets*mouse,*background,*window;				//�������ֲ�������ڴ棬ֻ����ʹ��ʱ�Ż�����ڴ棨����
	shectl->num=-1;
	shectl->top=0;							//ͼ��������ʼ��Ϊ-1
	shectl->map=alloc(info->xnumber*info->ynumber);
	
	mousebuf=alloc(16*16);						//��ȡ���ͼ������
	displaymouse(0,0,99,mousebuf,16);				//����͸��ɫΪ99
	mouse=shealloc(mousebuf,16,16,99);				//��ȡ���ͼ�㣨0�ţ�		
	mouse->height=0;						//���������ʾ�߶�
	shectl->sheetaddr[0]=mouse;					//�����ͼ��Ǽ�����ʾͼ��������
	slide(mouse,info->xnumber/2,info->ynumber/2);			//��������ʼ������
	
	backgroundbuf=alloc(info->xnumber*info->ynumber);		//��ȡ����ͼ������
	initscreen(backgroundbuf);					//������ͼ��д���ʼ��ͼ��
	background=shealloc(backgroundbuf,info->xnumber,info->ynumber,-1);	//��ȡ���汳��ͼ�㣨1�ţ�
	
	windowbuf=alloc(160*68);					//��ȡ��������
	window=shealloc(windowbuf,160,68,-1);				//��ȡ����ͼ�㣨2�ţ�
	displaywindow(window,"timer",0);				//�򴰿�ͼ��д���ʼ��ͼ��
	slide(window,80,72);						//���ô��ڳ�ʼ������
	
	refreshmap(0,info->xnumber,0,info->ynumber,shectl->top);
	top(background);						//�ö�����
	top(window);
}

struct sheets*shealloc(unsigned char*buf,int xsize,int ysize,long color)	//�½�ͼ���ӳ��򣬷���ͼ���ַ
{		//ͼƬ��ַ	//ͼƬ�������С//�������С//͸��ɫɫ��
	shectl->num++;									//��ͼ������һ
	if(shectl->num<MAXSHEET*2)
	{
		shectl->sheet[shectl->num]=allocbyte(LEN);				//ÿ�½�һ��ͼ������ڴ�����(��������Ŀռ��С����allocbyte)
		shectl->sheet[shectl->num]->buf=buf;					//�����β��趨ͼ����Ϣ
		shectl->sheet[shectl->num]->color=color;
		shectl->sheet[shectl->num]->x=0;					//���ó�ʼ������
		shectl->sheet[shectl->num]->y=0;
		shectl->sheet[shectl->num]->xsize=xsize;
		shectl->sheet[shectl->num]->ysize=ysize;
		shectl->sheet[shectl->num]->height=-1;					//���ø߶�Ϊ-1�����أ�
		shectl->sheet[shectl->num]->num=shectl->num;				//��¼��ͼ���ǵڼ���ͼ��
		shectl->sheet[shectl->num]->fifo=0;
		return shectl->sheet[shectl->num];					//����ͼ���ַ
	}
	return 0;									//���ͼ�����������ʧ�ܣ�����0
}

void shefree(struct sheets*sheet)			//�ͷ�ͼ���ӳ���
{		//ͼ���ַ
	long c;
	hide(sheet);								//�Ƚ���ͼ������
	for(c=0;c<sheet->fifo->task->appsheet.num;c++)				//�����ͼ����Ӧ�ó��򴰿�
		if(sheet==sheet->fifo->task->appsheet.sheet[c])			//�ڸ�������ʹ�õ�Ӧ�ó���ͼ�������н���ɾ�������ǣ�
		{
			for(;c<sheet->fifo->task->appsheet.num;c++)
				sheet->fifo->task->appsheet.sheet[c]=sheet->fifo->task->appsheet.sheet[c+1];
			sheet->fifo->task->appsheet.num--;			//�����������е�app������һ
			break;
		}					
	for(c=sheet->num;c<shectl->num;c++)			//һ��ͼ���ͷţ�����ͼ���Ժ������ͼ��ǰ�ƣ�����ͼ����Ϣ�ӱ�����Ƴ���
		shectl->sheet[c]=shectl->sheet[c+1];		//(�������Ĩȥͼ����Ȼ���㵫�ǿ��ܻᱻ�ڿ�����ɾȥ����ͼ��)����Ϊ��ȫ��Ӧ���ü�������
	shectl->num--;									//��ͼ������һ
	freebyte(sheet,LEN);								//���ڴ����ͷŸÿռ�
}

void top(struct sheets*sheet)							//��ͼ���ö������ͼ����⣩�ӳ���		
{	//ͼ���ַ
	long c;
	if(sheet->height>-1)								//�����ǰͼ���Ѿ�����ʾ��
	{	
		for(c=sheet->height;c>1;c--)						//ͼ��߶ȼ���ͼ����������ʾ��ͼ�������еı��
		{
			shectl->sheetaddr[c]=shectl->sheetaddr[c-1];			//����ͼ��ǰ������ͼ�㣨����⣩����
			shectl->sheetaddr[c]->height++;					//���ƶ���ͼ��߶ȼ�һ
		}
		shectl->sheetaddr[1]=sheet;						//����ǰͼ���ַ���õ���ʾͼ������ĵ�һ��
		sheet->height=1;							//��ͼ��ĸ߶���Ϊ1
		if(shectl->top>2)							//�����ʾ��ͼ����������2˵����������������
		{										//���⽫����ͼ��Ϳ�ϴ��ڱ�����
			wintitle(shectl->sheetaddr[2],1);				//���ոձ��µ��Ĵ��ڵı�����Ϳ��
			if(shectl->sheetaddr[2]->fifo)
				writebuf(shectl->sheetaddr[2]->fifo,-2);		//֪ͨ���µ��Ĵ���
		}
		if(shectl->top>1)							//�����ʾ��ͼ�����1˵��������һ��ͼ��
		{
			wintitle(sheet,0);						//���ö��Ĵ��ڵı�����Ϳ��
			if(shectl->sheetaddr[1]->fifo)
				writebuf(shectl->sheetaddr[1]->fifo,-1);		//֪ͨ���ö��Ĵ���
		}	
	}
	else
	{
		sheet->height=1;							//����ͼ��߶���Ϊ1
		shectl->top++;								//����ʾ��ͼ������һ
		if(shectl->top>=MAXSHEET)						//����ʾ��ͼ��������������ײ���ͼ��
			shectl->top=MAXSHEET-1;
		if(shectl->top>1)
			for(c=shectl->top;c>1;c--)						//��������ʾ��ͼ�㣨������⣩����
			{
				shectl->sheetaddr[c]=shectl->sheetaddr[c-1];
				shectl->sheetaddr[c]->height++;					//��ʾͼ����ƶ���Ҫ������ͼ��߶ȵĸı�
			}			
		shectl->sheetaddr[1]=sheet;						//����ͼ���ַ�Ǽ����ö�������꣩��ͼ��λ��
		if(shectl->top>2)							//�����ʾ��ͼ����������2˵����������������
		{										//���⽫����ͼ��Ϳ�ϴ��ڱ�����
			wintitle(shectl->sheetaddr[2],1);				//���ոձ��µ��Ĵ��ڵı�����Ϳ��
			if(shectl->sheetaddr[2]->fifo)
				writebuf(shectl->sheetaddr[2]->fifo,-2);		//֪ͨ���µ��Ĵ���
		}
		if(shectl->top>1)							//�����ʾ��ͼ�����1˵��������һ��ͼ��
		{
			wintitle(sheet,0);						//���ö��Ĵ��ڵı�����Ϳ��
			if(shectl->sheetaddr[1]->fifo)
				writebuf(shectl->sheetaddr[1]->fifo,-1);		//֪ͨ���ö��Ĵ���
		}
	}	
	winnow=sheet;									//�����ö�ͼ��ĵ�ַ������winnowָ���ö�ͼ�㣩
	refreshmap(0,info->xnumber,0,info->ynumber,shectl->top);			//ͼ��߶ȷ����仯ʱ��Ҫ����map��ǵ㶼ˢ��
	refresh(shectl->sheetaddr[2]->x+3,shectl->sheetaddr[2]->x+shectl->sheetaddr[2]->xsize-3,shectl->sheetaddr[2]->y+3,shectl->sheetaddr[2]->y+3+18);
	refresh(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize);		//ֻˢ����ʾ�ö�ͼ��
}

void hide(struct sheets*sheet)							//��ͼ�������ӳ���
{	//ͼ���ַ
	long c;
	if(sheet->height>-1)								//����ò�����ʾ
	{		
		for(c=sheet->height;c<shectl->top;c++)					//���ò����ʾͼ�������г�ȥ
		{
			shectl->sheetaddr[c]=shectl->sheetaddr[c+1];			//��ͼ���Ժ������ͼ��ǰ��
			shectl->sheetaddr[c]->height--;					//�ƶ���ͼ��߶ȼ�һ
		}			
		shectl->top--;								//��ʾ��ͼ������һ
		sheet->height=-1;							//��ͼ��߶���Ϊ-1
	}
	refreshmap(0,info->xnumber,0,info->ynumber,shectl->top);			//ͼ��߶ȷ����仯ʱ��Ҫ����map��ǵ㶼ˢ��
	refresh(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize);		//����ͼ������ˢ��
	top(shectl->sheetaddr[1]);							//�����غ��λ����ʾͼ���������λͼ���ö���ʵ�ֹر�����ʱ�Զ��л�����һ���ö����Ĵ��ڣ�
}

void slide(struct sheets*sheet,int x0,int y0)					//�ƶ�ͼ���ӳ���
{	//ͼ���ַ	   //Ŀ������
	int oldx,oldy;
	oldx=sheet->x;
	oldy=sheet->y;
	sheet->x=x0;								//����λ�Ƽ�����Ե�ַ
	sheet->y=y0;
	if(sheet->height>-1)							//�����ͼ��������ʾ��ˢ��
	{
		refreshmap(oldx,oldx+sheet->xsize,oldy,oldy+sheet->ysize,shectl->top);	    //ֻˢ��ƽ�Ƶ�ͼ�����ڵ�map��ǵ㣨ԭλ����ͼ�㶼Ҫ���£�
		refreshmap(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize,1);	//�ƶ����λ��ֻ��ˢ���ö�ͼ������ͼ�㼴��
		refresh(oldx,oldx+sheet->xsize,oldy,oldy+sheet->ysize);		//ֻˢ��ƽ�Ƶĵط���ͬ�ϣ�
		refresh(sheet->x,sheet->x+sheet->xsize,sheet->y,sheet->y+sheet->ysize);
	}
}

void refresh(int x0,int x1,			int y0,int y1)				//ˢ����ʾͼ���ӳ���
{	//Ҫˢ�µ�������vram�е�ʼĩ������//��������
	struct sheets*she;							//�ݴ�Ҫ��ʾ��ͼ��
	long c,color;
	int x,y;
	int dx0,dx1,dy0,dy1;
	if(x0<0)							//����������ѭ���г��ֽ϶๤��
		x0=0;						//�����Ͻ����곬���߿�ʱ����������ˢ�·�Χ��������Ļ
	if(y0<0)
		y0=0;
	if(x1>info->xnumber)							//����������ѭ���г��ֽ϶๤��
		x1=info->xnumber;						//�����½����곬���߿�ʱ����������ˢ�·�Χ��������Ļ
	if(y1>info->ynumber)
		y1=info->ynumber;
	for(c=shectl->top;c>-1;c--)
	{
		she=shectl->sheetaddr[c];						//��ȡҪ��ʾ��ͼ���е�ͼ��ĵ�ַ				
		dx0=x0-she->x;							//�������������ϵľ��������������Ը�ͼ���׵�ַ������
		dy0=y0-she->y;							//����vram�ϵľ�������ת��Ϊ��ͼ��ͼ�������е��������
		dx1=x1-she->x;
		dy1=y1-she->y;
		if(dx0<0)
			dx0=0;
		if(dy0<0)
			dy0=0;
		if(dx1>she->xsize)
			dx1=she->xsize;
		if(dy1>she->ysize)					//���ø�ͼ��߽������ʾ��Χ
			dy1=she->ysize;
									//���û���غϲ��֣���ôx<dx1����������ֱ��������ִ��
										//�ʿ���ʡ���ж������غϲ��ֵ����
		for(x=dx0;x<dx1;x++)					//��ʾ��ͼ����Ҫˢ�µĲ��֣�x,yΪ�ڸ�ͼ��ͼ�������е�λ�ã�		
			for(y=dy0;y<dy1;y++)			
			{
				if(shectl->map[x+she->x+(y+she->y)*info->xnumber]!=she->height)		//����Ǹ�ͼ���λ�þ�д������
					continue;
				color=she->buf[x+y*she->xsize];			//ȡ���صķ���Ҫ��д���صķ���һ�£��˴���Ϊ����
				info->vram[x+she->x+(y+she->y)*info->xnumber]=color;	
			}
				
																
	}	
}

void refreshmap(int x0,int x1,			int y0,int y1,unsigned char h)		//���ͼ��λ���ӳ���
{		//Ҫˢ�µ�������vram�е�ʼĩ������//��������//��ʾ����ײ��ͼ��߶�
	struct sheets*she;							//�ݴ�Ҫ��ʾ��ͼ��
	long c,color;
	int x,y;
	int dx0,dx1,dy0,dy1;
	if(x1>info->xnumber)							//����������ѭ���г��ֽ϶๤��
		x1=info->xnumber;						//�����½����곬���߿�ʱ����������ˢ�·�Χ��������Ļ
	if(y1>info->ynumber)
		y1=info->ynumber;
	for(c=h;c>-1;c--)
	{
		she=shectl->sheetaddr[c];						//��ȡҪ��ʾ��ͼ���е�ͼ��ĵ�ַ				
		dx0=x0-she->x;							//�������������ϵľ��������������Ը�ͼ���׵�ַ������
		dy0=y0-she->y;							//����vram�ϵľ�������ת��Ϊ��ͼ��ͼ�������е��������
		dx1=x1-she->x;
		dy1=y1-she->y;
		if(dx0<0)
			dx0=0;
		if(dy0<0)
			dy0=0;
		if(dx1>she->xsize)
			dx1=she->xsize;
		if(dy1>she->ysize)					//���ø�ͼ��߽������ʾ��Χ
			dy1=she->ysize;
									//���û���غϲ��֣���ôx<dx1����������ֱ��������ִ��
		if(she->color==-1)					//���û��͸��ɫ
			for(x=dx0;x<dx1;x++)					//��ʾ��ͼ����Ҫˢ�µĲ��֣�x,yΪ�ڸ�ͼ��ͼ�������е�λ�ã�		
				for(y=dy0;y<dy1;y++)	
				{
					color=she->buf[x+y*she->xsize];				
					shectl->map[x+she->x+(y+she->y)*info->xnumber]=c;	
				}	
		else
			for(x=dx0;x<dx1;x++)					//��ʾ��ͼ����Ҫˢ�µĲ��֣�x,yΪ�ڸ�ͼ��ͼ�������е�λ�ã�		
				for(y=dy0;y<dy1;y++)	
				{
					color=she->buf[x+y*she->xsize];	
					if(color!=she->color)			//�������͸��ɫ�򽫸�ͼ��߶�д��map				
						shectl->map[x+she->x+(y+she->y)*info->xnumber]=c;	
				}														

	}
}
