#include"io.h"
#include"init.h"

extern struct shectls*shectl;
struct mesg*info=(struct mesg *)ADR_MESG;
	
static unsigned char rgb[16*3]=			//���õ�ɫ�����ݿ�
{						/*��Ϊ��̬��������ʾֱ�Ӷ������ݶ�����һ��һ����ֵ�Խ�ʡ�����ֽ���*/
	0x00, 0x00, 0x00,			/*  0:�\ 		*/
	0xff, 0x00, 0x00,			/*  1:����		*/		/*�����е�ע��Ҫ��*��Ȼ���ݻ���� */
	0x00, 0xff, 0x00,			/*  2:���v 		*/
	0xff, 0xff, 0x00,			/*  3:����		*/
	0x00, 0x00, 0xff,			/*  4:����		*/
	0xff, 0x00, 0xff,			/*  5:���� 		*/
	0x00, 0xff, 0xff,			/*  6:ǳ����		*/
	0xff, 0xff, 0xff,			/*  7:�� 		*/
	0xc6, 0xc6, 0xc6,			/*  8:���� 		*/
	0x84, 0x00, 0x00,			/*  9:���� 		*/
	0x00, 0x84, 0x00,			/*  10:���� 		*/
	0x84, 0x84, 0x00,			/*  11:���� 		*/
	0x00, 0x00, 0x84,			/*  12:���� 		*/
	0x84, 0x00, 0x84,			/*  13:���� 		*/
	0x00, 0x84, 0x84,			/*  14:ǳ���� 		*/
	0x84, 0x84, 0x84			/*  15:���� 		*/
};
 
void initscreen(char*p)				//��ʼ����Ļ��ʾ�ӳ���
{	
	squer(0,info->xnumber,0,info->ynumber,p,info->xnumber,BACKCOL);	//��VRAM�Դ�д��������ɫ	//����		//��������assembly.nas��ͷ�����ģʽ��Ϣ��ȡ��
	squer(0,80,info->ynumber-40,info->ynumber,p,info->xnumber,4);      //UI		//��ʹϵͳ������ģʽ��Ҳ����������
	squer(info->xnumber-80,info->xnumber,info->ynumber-40,info->ynumber,p,info->xnumber,4);		//UI
	squer(1,80-1,info->ynumber-40+1,info->ynumber-1,p,info->xnumber,BACKCOL);		//UI
	squer(info->xnumber-80+1,info->xnumber-1,info->ynumber-40+1,info->ynumber-1,p,info->xnumber,BACKCOL);		//UI
}

void initpalette(int start,int end)		//���õ�ɫ��ɫ���ӳ���
{		//Ҫ���õĵ�ɫ����ʼ���
	int eflag,i=0;				//i��rgbԪ�ر��
	unsigned char c;			//ɫ��c���Ϊ255
	unsigned char table2[216 * 3];		//ɫ�׷�
	int r, g, b;	
	eflag=io_popeflag();			//�����ʱ�ı�־�Ĵ���״̬
	io_cli();				//�ر��жϱ�־λ
	io_out8(0x3c8,start);			//��0x3c8����Ҫ�趨�ĵ�ɫ���ţ�����ʡ�Ըò���ʾ�趨��һ����ţ�����һ������ʡ�ԣ�
	for(c=start;c<=end;c++)		//��0x3c9�ֱ�����RGB���趨�ñ�ŵ���ɫ״̬
	{
		io_out8(0x3c9,rgb[i]/4);	//��0x3c9�˿����R
		io_out8(0x3c9,rgb[i+1]/4);	//��0x3c9�˿����G
		io_out8(0x3c9,rgb[i+2]/4);	//��0x3c9�˿����B
		i+=3;
	} 
	
	for (b = 0; b < 6; b++) 		//����Ϊɫ�׷���ɫ 
		for (g = 0; g < 6; g++) 
			for (r = 0; r < 6; r++) 
			{
				table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
				table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
	for(c=16;c<=231;c++)		//��0x3c9�ֱ�����RGB���趨�ñ�ŵ���ɫ״̬
	{
		io_out8(0x3c9,table2[i]/4);	//��0x3c9�˿����R
		io_out8(0x3c9,table2[i+1]/4);	//��0x3c9�˿����G
		io_out8(0x3c9,table2[i+2]/4);	//��0x3c9�˿����B
		i+=3;
	} 
	io_pusheflag(eflag);			//�ָ���־�Ĵ���״̬
}

void squer(int x0,int x1,int y0,int y1,  		 char*p,   short xnumber,unsigned char color)		//��ʾ�����ӳ���	
{	//������ʼĩ��ַ//y����ʼĩ��ַ�������д��ͼ��//Ҫд��ͼ����׵�ַ//������������//������ɫ						
	int x,y;					
	for(x=x0;x<x1;x++)				//*P������Ϊָ��char�Ͳ����ڼ�1��ָ����һ���ֽڵ�ַ����һ�����أ�����Ϊint���Ǹ�1����������ʾ
		for(y=y0;y<y1;y++)			//*((int*)0x0ff8)����p����Ҫ�������ֽڣ�char*p����p[x+y*xnumber]=color��Ҫ��Ϊһ���ֽ�һ���ֽڵؼ�
			p[x+y*xnumber]=color;		//*(p+1)���+1�Ǽ�1�ֽڻ��Ǽ�1��ȡ����pָ������ͣ����˴���char*p����p�Ѿ��ڱ���ֵʱ�ͱ�����Ϊ�����ͣ�
}

void displaymouse(int x,  int y,unsigned char color,char*p,short xnumber)				//��ʾ�����״�ӳ���
{	//��������ʼ��ַ//y������ʼ��ַ//����͸��ɫ//�Դ��׵�ַ//���������������
	static unsigned char cursor[16][16] = 			/*���������״�����ݿ�*/
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
	unsigned char a,b;						//�����±겻��Ϊ����
	p+=x+y*xnumber;	
	for(a=0;a<16;a++)						//0-15��16λ��ʹ��for������ʱҪ�ر�ע�⣩����Ϊ<=16������������ʾ����
		for(b=0;b<16;b++)
			switch(cursor[a][b])
			{
			case'*':p[a*xnumber+b]=0;break;			//��*���ֽ���ʾ��ɫ���߿�
			case'O':p[a*xnumber+b]=7;break;			//��0���ֽ���ʾҪ�����ɫ
			default:p[a*xnumber+b]=color;			//����������������ɫ�����ñ���ɫ
			}
}

void printc(unsigned char a[16],  int x,   int y,    unsigned char color,  char*p,    short xnumber)	//��ʾ�����ַ��ӳ���
{	//Ҫ��ʾ�ַ������ݿ�//��ʼ��ʾ�ַ���������꣨x��y��//�ַ���ɫ//vram�Դ��׵�ַ//������������					
	unsigned int i,o;
	int byte;
	p+=x+y*xnumber;					//p=��ʼ�������ݵ��׵�ַ
	for(i=0;i<16;i++)				//��16������Ҫ���루��������16λ����16*8���������ص�����ʾ�ַ���
	{
		byte=(int)a[i];				//�����ݿ�ȡ���ַ�
		for(o=0;o<8;o++)			//ÿ��������8λ(��������8λ)
		{
			if(byte&0x80)			//��byte�ĵڰ�λ�Ƿ�Ϊ1����Ϊ1��øı���ɫ
				p[i*xnumber+o]=color;	//i�൱��������仯��o�൱�ں�����仯
			byte=io_shl(byte);		//��byte����һλ��������һ���жϣ�������λ�����ڰ�λ��
		}		
	}	
}

void prints(char*c,  		int x,   int y,   unsigned char color,char*p,short xnumber)				//��ʾ�ַ����ӳ���
{	//�ַ���//��ʼ��ʾ�ַ��������Ͻ����꣨x��y��//�ַ���ɫ//�ַ�д���ͼ���׵�ַ//��ͼ�������������
	for(;*c!=0;c++,x+=8)						//һֱ��ʾֱ���ַ���������־��\0����ÿ��ʾһ���ַ�����ַ��һָ����һ���ַ����������8��ͬһ����ʾ�¸��ַ�
		printc(hankaku+*c*16,x,y,color,p,xnumber);
}

void displaywords(unsigned char*s,int x0,int y0,    	char num,struct sheets*she,unsigned char fcolor,unsigned char bcolor)
{		//�ַ���//��Ҫд��ͼ���е�λ�õ����Ͻ�����//�ַ�����//�ַ�д���ͼ���ַ	//�ַ���ɫ	//�ַ�����ɫ
	squer(x0,x0+num*8,y0,y0+16,she->buf,she->xsize,bcolor);
	prints(s,x0,y0,fcolor,she->buf,she->xsize);
	refresh(x0+she->x,x0+num*8+she->x,y0+she->y,y0+16+she->y);				//�������λ�ü������vram�еľ���λ�ú����ˢ��
}				//���������㵫�����յ�

void wintitle(struct sheets*sheet,	char m)
{		//��������ͼ���ַ//�������Ǻڣ�0��������ң���0��
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
	squer(3,sheet->xsize-3,3,3+18,sheet->buf,sheet->xsize,tc); 				//��������ɫ
	prints(sheet->title,4,4,7,sheet->buf,sheet->xsize);					//����������
	for (y = 0; y < 14; y++) 					//�رհ�ťͼ��
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
{		//��������ͼ���ַ//��������//�������Ǻڣ�0��������ң���0��
	squer(0,sheet->xsize-1,0,1,sheet->buf,sheet->xsize,7);						//��ɫ�ϱ߿�
	squer(0,1,1,sheet->ysize-1,sheet->buf,sheet->xsize,7);						//��ɫ��߿�
	squer(0,sheet->xsize,sheet->ysize-1,sheet->ysize,sheet->buf,sheet->xsize,0);			//��ɫ�ұ߿�
	squer(sheet->xsize-1,sheet->xsize,sheet->ysize,sheet->ysize-1,sheet->buf,sheet->xsize,0);	//��ɫ�±߿�  ��������ӰЧ����
	
	squer(1,sheet->xsize-1,1,sheet->ysize-1,sheet->buf,sheet->xsize,15);				//�װ尵��    ����������Ч����
	squer(2,sheet->xsize-2,2,sheet->ysize-2,sheet->buf,sheet->xsize,8);				//������������
	
	sheet->title=title;
	wintitle(sheet,m);
}
