#include"init.h"

int *fat;									//������fat���ڴ��еĵ�ַ

void readfat()
{
	unsigned char *img=(unsigned char *)(0x00100000+0x000200);		//�������ڴ��еĵ�ַ+FAT�ڴ��̵ĵ�ַ
	fat=alloc(4*2880);							//���ڴ��п���24��������11KB�����������FAT
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) 						//��������P383
	{
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
}

void loadfile(int cluster, int size, char *buf)
{	//�ļ���ʼ���������//�ļ���С//������ļ����ڴ��ַ
	int i;
	unsigned char *img=(unsigned char *)(0x00100000+0x003e00);		//�������ڴ��еĵ�ַ+0�Ŵ��ڴ����е��׵�ַ
	while(1)									//һ���صĴ�С=һ�������Ĵ�С
	{
		if (size <= 512)						//�����ʣ�ࣩ�ļ���С��һ����֮��
		{
			for (i = 0; i < size; i++) 				
				buf[i] = img[cluster * 512 + i];		//�Ӹô��׵�ַ��ʼ���ļ����Ƶ��ڴ�
			break;
		}
		for (i = 0; i < 512; i++) 					//����ļ���ɢ�ڲ�ͬ��
			buf[i] = img[cluster * 512 + i];			//�Ƚ��ô��е����ݸ��Ƶ��ڴ�
		size -= 512;							//Ҫ���ƵĴ�С����512b��һ�������Ĵ�С��
		buf += 512;							//��ʼ������ڴ��ַ������512�ֽ�
		cluster = fat[cluster];						//��FAT�ж�ȡ���ļ�����һ������أ�P384��
	}
}
