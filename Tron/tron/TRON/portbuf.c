#include"init.h"

struct fifo32 mainfifo;
struct fifo32 mousefifo;

void initbuf(struct fifo32*fifo)			//��ʼ���������ӳ��򣨽��ܽṹ���ַ�����Ǵ�����ʱ�ṹ�������
{	//�������ṹ���ַ//���������ݴ��������ַ//��������С�������С��
	fifo->size=128;								//��ʼ����������С
	fifo->free=128;								//��ʼ��������ʣ���С
	fifo->write=0;								//��ʼ��������д��λ��
	fifo->read=0;								//��ʼ����������ȡλ��
	fifo->task=0;								//��ʼ��ָ������Ϊ��
}

int writebuf(struct fifo32*fifo,int data)			//�򻺳���д�������ӳ���
{
	if(fifo->free==0)							//����������������򷵻�-1
		return -1;
	fifo->buf[fifo->write]=data;						//������д�뻺����
	fifo->write++;								//д��λ������һλ(���ջ��ָ��)
	if(fifo->write==fifo->size)						//��д��λ�ô����������߽�ʱ�ص����
		fifo->write=0;
	fifo->free--;								//���㻺����ʣ���С
	if(fifo->task)								//�����ָ�����ѵ�����
		if(!fifo->task->status)						//���������������״̬
			taskwake(fifo->task);					//���Ѹ�����
	return 0;
}

int readbuf(struct fifo32*fifo)					//�ӻ��������������ӳ���
{
	int data;
	if(fifo->free==fifo->size)						//�����������û�������򷵻�-1
		return -1;
	data=fifo->buf[fifo->read];						//�����ݴӻ���������������data
	fifo->read++;								//����λ������һλ����д��λ�ö���������
	if(fifo->read==fifo->size)						//������λ�ô����������߽�ʱ�ص����
		fifo->read=0;
	fifo->free++;								//���㻺����ʣ���С
	return data;								//���ض���������
}

int bufstatus(struct fifo32*fifo)					//���滺�����ж��������ӳ���
{
	return fifo->size-fifo->free;
}

