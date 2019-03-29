#include"init.h"

int *fat;									//解码后的fat在内存中的地址

void readfat()
{
	unsigned char *img=(unsigned char *)(0x00100000+0x000200);		//磁盘在内存中的地址+FAT在磁盘的地址
	fat=alloc(4*2880);							//在内存中开辟24个扇区（11KB）储存解码后的FAT
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) 						//解码规则见P383
	{
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
}

void loadfile(int cluster, int size, char *buf)
{	//文件开始储存的扇区//文件大小//储存该文件的内存地址
	int i;
	unsigned char *img=(unsigned char *)(0x00100000+0x003e00);		//磁盘在内存中的地址+0号簇在磁盘中的首地址
	while(1)									//一个簇的大小=一个扇区的大小
	{
		if (size <= 512)						//如果（剩余）文件大小在一个簇之内
		{
			for (i = 0; i < size; i++) 				
				buf[i] = img[cluster * 512 + i];		//从该簇首地址开始将文件复制到内存
			break;
		}
		for (i = 0; i < 512; i++) 					//如果文件分散在不同簇
			buf[i] = img[cluster * 512 + i];			//先将该簇中的数据复制到内存
		size -= 512;							//要复制的大小少了512b（一个扇区的大小）
		buf += 512;							//开始储存的内存地址往后移512字节
		cluster = fat[cluster];						//从FAT中读取该文件的下一个储存簇（P384）
	}
}
