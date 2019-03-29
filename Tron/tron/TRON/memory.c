#include"io.h"
#include"init.h"

unsigned int memcount(unsigned int start,unsigned int end)		//�ڴ������������ӳ�������ܼ��1GB�ڴ棩
{
	char cpu486=0;						//����Ƿ���486�����ϵ�cpu
	unsigned int eflag,cr0,i;				//�����־�Ĵ�����cr0�Ĵ�����ֵ���ڴ��С��ֵ
					//����Ƿ���486�����ϵ�cpu
	eflag=io_popeflag();					//ȡ�õ�ǰ�ı�־�Ĵ�����ֵ
	eflag|=0x40000;						//486�����ϵ�cpu�ĵ�18λΪac��486���µ�û��acλ����ʹ������acλҲ��Ч���ʽ�acλ��Ϊ1
	io_pusheflag(eflag);					//���ñ�־�Ĵ���
	eflag=io_popeflag();					//���¶�ȡ��־�Ĵ���
	if(eflag&0x40000)					//���acλ����acΪ1˵��Ϊ486�����ϡ���Ϊ0��Ϊ486����
		cpu486=1;
	eflag&=~0x40000;					//�ָ��Ĵ���ԭ����ֵ
	io_pusheflag(eflag);
					//�����486�����ϵ�cpu����Ҫ���λ������׼ȷ��ֱ�ӹ�ͨ�ڴ�
	if(cpu486)						
	{
		cr0=io_loadcr0();
		cr0|=0x60000000;
		io_storecr0(cr0);
	}
					//ȡ���ڴ�����
	i=io_count(start,end);					
					//�����486�����ϵ�cpu�����¿�������
	if(cpu486)						
	{
		cr0=io_loadcr0();
		cr0&=~0x60000000;
		io_storecr0(cr0);
	}
					//�����ܹ�ʹ�õ��ڴ�����ĵ�ַ
	return i;
}

struct manage*memory=(struct manage*)0x003c0000;				//���ڴ���������0x003c0000��

void initmemory()						//��ʼ���ڴ������ӳ���
{
	memory->block=0;
	memory->freememo=0;	
	memory->lost=0;	
	memory->lostsize=0;							//������ָ����Ϊ0
	free((void*)0x00001000,0x0009e000+1);					//�Ǽ�0:1000��9:f000���ڴ�Ϊ���ã�vramΪa:0000��a:ffff,b:0000��Ϊbios��
	free((void*)0x00400000,memcount(0x00400000,0xbfffffff)-0x00400000+1);	//�Ǽ�0040:0000��0200:0000���ڴ�Ϊ����(�ڴ��С�����������)
}

void*allocbyte(unsigned int size)				//�����ڴ��ӳ��򣬷��������ڴ���׵�ַ
{	//�����ڴ��С
	unsigned int c;						//�ڴ�������
	void *addr;						//�ݴ��׵�ַ
	for(c=0;c<memory->block;c++)				//�������ڴ�������޷����������ڴ�����
		if(size<=memory->memo[c].size)			//�����
		{
			addr=memory->memo[c].addr;		//�����ڴ���׵�ַ����addr
			memory->memo[c].addr+=size;		//���ڴ���׵�ַ����
			memory->memo[c].size-=size;		//�ڴ��Ŀ����ڴ��С����
			memory->freememo-=size;			//�ܿ����ڴ����
			if(!memory->memo[c].size)		//������ڴ���СΪ0
			{
				memory->block--;		//ȥ�����ڴ��
				for(;c<memory->block;c++)	//���ڴ��֮����ڴ��ǰ�Ƹ���
					memory->memo[c]=memory->memo[c+1];				
			}
			return addr;				//�����׵�ַ
		}
	return 0;						//���ڴ治���򷵻�0
}

void*alloc(unsigned int size)				//����4k�ڴ��ӳ���(��4kΪ��λ�����ڴ����)
{
	size=(size+0xfff)&0xfffff000;
	return allocbyte(size);
}

char freebyte(void*addr,		unsigned int size)	//�ڴ��ͷ��ӳ��򣬳ɹ�����0ʧ�ܷ���-1
{	//Ҫ�ͷ��ڴ���׵�ַ   //Ҫ�ͷŵĴ�С
	int c,d;						//�ڴ�������
	for(c=0;c<memory->block;c++)				//�ڴ���Ѿ����׵�ַ��С��С�������У���Ҫ�ͷŵ��׵�ַС�������׵�ַʱ����ѭ��
		if(addr<memory->memo[c].addr)
			break;
	if(c>0)							//��֤[c-1]��Խ��
		if(memory->memo[c-1].addr+memory->memo[c-1].size==addr)  	//��Ҫ�ͷŵ��׵�ַ��ǰ��һ���ڴ��������һ��ʱ
		{
			memory->memo[c-1].size+=size;				//ǰһ�ڴ������ڴ��С����
			memory->freememo+=size;					//�ܿ����ڴ�����
			if(addr+size==memory->memo[c].addr)			//���Ҫ�ͷŵ��׵�ַ�����һ�ڴ��������һ��
			{
				memory->memo[c-1].size+=memory->memo[c].size;	//����һ�ڴ��Ĵ�С���ӵ�ǰһ�ڴ����
				memory->block--;				//�ڴ����������
				for(;c<memory->block;c++)			//�ڴ��ǰ�Ƹ���
					memory->memo[c]=memory->memo[c+1];
			}
			return 0;						
		}
	if(addr+size==memory->memo[c].addr)			//��Ҫ�ͷŵ��׵�ַ������ǰһ�ڴ������һ���ܺͺ�һ���ڴ��������һ��ʱ
	{
		memory->memo[c].addr=addr;			//��һ�ڴ���׵�ַ��ΪҪ�ͷ��ڴ���׵�ַ
		memory->memo[c].size+=size;			//��һ�ڴ������ڴ��С����
		memory->freememo+=size;				//�ܿ����ڴ�����
		return 0;
	}			
	if(memory->block<BLOCKNUM)				//���ǰ�󶼲����������ڴ�������������С��Χ��
	{
		for(d=memory->block;d>c;d--)			//�Ӻ�һ�ڴ�飨������ʼȫ���ڴ�����
			memory->memo[d]=memory->memo[d-1];
		memory->memo[c].addr=addr;			//��Ҫ�ͷŵ��ڴ����Ϣд�ڡ���һ���ڴ��ԭ����λ��
		memory->memo[c].size=size;
		memory->freememo+=size;				//�ܿ����ڴ�����
		memory->block++;				//�ڴ����������
		return 0;
	}
	else							//����ڴ��������
	{
		memory->lostsize+=size;				//����ǰҪ�ͷŵ��ڴ���������¼���������ڴ��С
		memory->lost++;					//��¼�����ڴ�Ĵ���
		return -1;
	}	
}

char free(void*addr,unsigned int size)			//4k�ڴ��ͷ��ӳ���
{
	size=(size+0xfff)&0xfffff000;
	return freebyte(addr,size);
}
