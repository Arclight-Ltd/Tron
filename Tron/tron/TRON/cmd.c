#include"init.h"
#include"io.h"
#include<stdio.h>
#include<string.h>

extern struct shectls*shectl;	
extern struct manage* memory;
extern struct appsheets appsheet;
extern struct sheets*winnow;						//��ǰѡ�еĴ���

struct fileinfos
{
	unsigned char name[8], ext[3], type;				//8�ֽ��ļ��������㲹�ո񣩣���չ�����ļ����ԣ�P367��
	char reserve[10];						//�����ռ�
	unsigned short time, date, cluster;				//�ļ�����ʱ�䣬���ڣ��ļ�����������أ�
	unsigned int size;						//�ļ���С
};
struct fileinfos*fileinfo=(struct fileinfos*)ADR_FILEINFO;

/**************************************************************************************************************************************/

void error(char*cmdws)									//����ָ��ִ�е��ӳ���
{
	putstr0("Sorry,I don't understand\n\n");
}

void name(char*cmdws)									//ѯ��ϵͳ����
{
	putstr0("My name is Tron\n\n");
}

void mem(char*cmdws)									//ѯ�ʵ�ǰ�ڴ�ʹ��״��
{
	char*s=0;
	sprintf(s,"I have %dMB memery in total\n",memcount(0x00400000,0xffffffff)/(1024*1024));
	putstr0(s);
	sprintf(s,"But now I have only %dKB in free\n\n",memory->freememo/1024);
	putstr0(s);
}

void cls(char*cmdws)									//�����������д��ڣ�
{
	struct conchar*conc=&winnow->fifo->task->conc;
	squer(4,256*2-3,24,165*2-5,shectl->sheet[3]->buf,shectl->sheet[3]->xsize,0);
	refresh(4+shectl->sheet[3]->x,256*2-3+shectl->sheet[3]->x,24+shectl->sheet[3]->y,165*2-5+shectl->sheet[3]->y);
	conc->cursor_x=1;
	conc->cursor_y=0;
}

void dir(char*cmdws)									//��ʾϵͳ�ļ���Ϣ
{
	char*s=0;
	unsigned char x,y;
	putstr0("  filename            size\n");
	for(x=0;x<224;x++)
	{
		if(fileinfo[x].name[0]==0)				//�ļ�����һ���ֽ�Ϊ0˵����һ�β������κ��ļ�����Ϣ
			break;
		if(fileinfo[x].name[0]!=0xe5)				//�ļ�����һ���ֽ�Ϊ0xe5˵�����ļ���ɾ��
			if((fileinfo[x].type&0x18)==0)		//�ų�Ŀ¼�ͷ��ļ���Ϣ�Ķ�
			{
				sprintf(s,"filename.ext       %7dB\n",fileinfo[x].size);	//¼���ļ���С
				for(y=0;y<8;y++)			//¼���ļ���
					s[y]=fileinfo[x].name[y];
				s[9]=fileinfo[x].ext[0];		//¼����չ��
				s[10]=fileinfo[x].ext[1];
				s[11]=fileinfo[x].ext[2];
				putstr0(s);
			}
	}
	putchar('\n');						//""���ø��ַ����׵�ַ��ֵ����������ֱ�ӽ����ַ���ascll����Ϊʵ��
}

void run(char*cmdws)									//����Ӧ�ó���run+�ո�+�ļ�����
{
	char*file,*filestack,*po;
	char name[11];
	unsigned char x;
	int c;
	int segsize,datasize,esp,datahrb;
	struct tasks*task;
	for(x=0;x<8;x++)						//�ÿո����name
		name[x]=0x20;
	po=strstr(cmdws,"RUN");						//�ҵ�run���ַ����е�λ��
	for(x=4;(cmdws[x]!=0)&&(x<8+4)&&(po[x]!='.');x++)		//ֻ��run����ļ�������name
		name[x-4]=po[x];
	name[8]='H';
	name[9]='R';
	name[10]='B';							//Ĭ���ļ���׺����ִ�������ļ�
	for(x=0;x<224;x++)						//�����ļ�
	{
		if(fileinfo[x].name[0]==0)				//�ļ�����һ���ֽ�Ϊ0˵����һ�β������κ��ļ�����Ϣ
			break;
		if(fileinfo[x].name[0]!=0xe5)				//�ļ�����һ���ֽ�Ϊ0xe5˵�����ļ���ɾ��
			if((fileinfo[x].type&0x18)==0)			//�ų�Ŀ¼�ͷ��ļ���Ϣ�Ķ�
				if(!memcmp(fileinfo[x].name,name,11))	//���name�ж�ȡ���ļ��ʹ����е��ļ�����ȫ�������������õĿո�
					break;				//����for
	}
	if((x<224)&&(fileinfo[x].name[0]!=0))				//���������ΪxԽ���name[0]=0���µ�������˵���ҵ��˿���ִ�е��ļ���
	{
		file=alloc(fileinfo[x].size);				//���ڴ��п���һ����ʱ�Ŀռ��Ÿ�Ӧ�ó���
		task=winnow->fifo->task;		
		loadfile(fileinfo[x].cluster,fileinfo[x].size,file);	//���ó��������شӴ����ж����ڴ�
											
		if(fileinfo[x].size>=36&&memcmp(file+4,"Hari",4)==0&&file[0]==0)//�����ִ���ļ���ͨ��bim2hrb���ɵ�,��һ���ֽ�һ��Ϊ0�����ĸ��ֽڿ�ʼһ��ΪHari
		{
			segsize=*((int*)(file+0x00));				//Ӧ�ó����������ݶδ�С(�����fileת��Ϊint*�Ͳ���һ�ζ�ȡ4���ֽڣ���Ȼֻ�ǰ�һ���ֽڸ�ֵ��ȥ������)
			esp=*((int*)(file+0x0c));				//ESP��ʼֵ����ǰΪջ����Ϊ�ڴ����ݶΣ�
			datasize=*((int*)(file+0x10));				//Ӧ�ó������ݲ��ִ�С
			datahrb=*((int*)(file+0x14));				//Ӧ�ó������ݲ����׵�ַ
			filestack=alloc(segsize);				//ΪӦ�ó��򿪱��µ����ݿռ�
			*((int*)0xfe8)=(int)filestack;				//��Ӧ�ó�������ݶ����ڴ��е���ʱ�׵�ַ������0xfe8��															
			for(c=0;c<datasize;c++)
				filestack[esp+c]=file[datahrb+c];		//��Ӧ�ó�������ݲ��ָ��Ƶ�Ӧ�ó�������ݶ�
			setseg(task->ldt+0, fileinfo[x].size-1,(int)file, CODE32+0x60);	//���ø�Ӧ�ó����ָ��Σ�������Ȩ��+0x60�ɽ�����ΪӦ�ó����ã��������ú�cs��Ӧ�ó��������ϵͳ���л�ʱss/dsҲ���Զ��л���������tss�Ĺ������ƣ�����Ӧ�ó���Ǽ���LDT�ж�����GDT�У�������������Ӧ�ó��������е�ʱ��Ͳ��ܻ�����ʣ�����Ӧ�ó���
			setseg(task->ldt+1, segsize-1,(int)filestack, DATA32+0x60);	//���ø�Ӧ�ó�������ݶΣ�csָ��Ӧ�ó����ʱ���������ϵͳ�ν�������һ�㱣���쳣������Ȩ��+0x60�Ķ�ֻ�ܷ���Ȩ��ͬ������0x60�ĶΣ�		
											//��Ӧ�ó���ģʽ�У�in��out��cli��sti��hltָ��Ͷ�ϵͳ�εĲ�������ϵͳ�����call��jmp��ͨ��API���ó��⣩���ᱻ�϶�Ϊһ�㱣���쳣
			io_startapp(0x1b,0*8+4,esp,1*8+4,&(task->tss.esp0));	//����ת��ipΪ0x1b��ʼִ��.���ø��Ĵ�������ø�Ӧ�ó���+4��ʾ����LDT�еĶκţ����Զ���GDT���ҳ���ӦLDT�ĵ�ַ��������ÿ�������LDT���ǻ�������ģ�����ֲ����������Ծ���κ���ͬҲû�£���app��LDT�жκ���ͬ����LDT��GDT�еǼ�ʱ�ĶκŲ�ͬ��
			if(task->appsheet.num)					//Ӧ�ó���������������������ڣ��еĻ���һ�ͷ�
			{
				for(c=0;c<task->appsheet.num;c++)
					shefree(task->appsheet.sheet[c]);
			}
			if(task->apptimer.num)					//Ӧ�ó��������������������ʱ�����еĻ���һ�ͷ�
			{
				for(c=0;c<task->apptimer.num;c++)
					timerfree(task->apptimer.timer[c]);
			}
			free(filestack,segsize);
			putchar('\n');
		}	
		else								//�����ļ����ͷ��ʼִ��
			putstr0("It's not a .hrb file \n\n");		
		free(file,fileinfo[x].size);					//�ͷ���ʱ�ռ�
	}
	else									//���û�и��ļ�
		putstr0("File not found\n\n");
}

void opencmd(char*cmdws)							//���µ������д���		
{
	putstr0("new console opened\n\n");
	initconsole();
}

void exit(char*cmdws)								//�ر������д��ڲ��ͷ���������йص����пռ�	
{
	writebuf(winnow->fifo,KILL);						//֪ͨ�Լ���Ҫ���ͷ�
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
