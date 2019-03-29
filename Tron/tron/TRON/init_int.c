#include"io.h"
#include"init.h"

extern struct timerctls*timerctl;

void initGDTIDT(void)											//��ʼ��GDT��IDT�ӳ���
{
	struct segment_descriptor *gdt = (struct segment_descriptor *) ADR_GDT;			//GDT��ʼ��ַ0027:0000
	struct gate_descriptor    *idt = (struct gate_descriptor    *) ADR_IDT;			//IDT��ʼ��ַ0026:f800
	int i;	
	
	for (i = 0; i < 8192; i++)					//GDT��ʼ����0~8191���Σ�
		setseg(gdt + i, 0, 0, 0);					//��ÿ���εĵ�ַ���ޣ��յ�ַ������ַ��ʼ��ַ��������Ȩ�޳�ʼ��Ϊ0
	setseg(gdt + 1, 0xffffffff, 0x00000000, DATA32);			//����1�ŶΣ������ڴ棩
	setseg(gdt + 2, LIMIT_HARIMAIN, ADR_HARIMAIN, CODE32);			//����2�ŶΣ�����harimain.hrb��
	io_loadgdtr(LIMIT_GDT, ADR_GDT);					//����Ч�趨������GDT�ڴ���ʼ��ַ����CPU�ڵ�GDTR�Ĵ���

	for (i = 0; i < 256; i++)					//IDT��ʼ����0~255���жϣ�
		setgate(idt + i, 0, 0, 0);
		
	setgate(idt+0x0c,(int)int0ch,2<<3,INTGATE32);				//����0ch���ж�
	setgate(idt+0x0d,(int)int0dh,2<<3,INTGATE32);				//����0dh���ж�
	
	setgate(idt+0x20,(int)int20h,2<<3,INTGATE32);				//����20h���ж�
	setgate(idt+0x21,(int)int21h,2<<3,INTGATE32);				//����21h���ж�
	setgate(idt+0x27,(int)int27h,2<<3,INTGATE32);				//����27h���ж�
	setgate(idt+0x2c,(int)int2ch,2<<3,INTGATE32);				//����2ch���ж�
	
	setgate(idt+0x40,(int)int40h,2<<3,INTGATE32+0x60);			//����40h���ж�Ϊ�ɹ�Ӧ�ó�����ã���ʾ���ж��ǿɹ�Ӧ�ó�����õ�API����Ӧ�ó���ֻ�ܵ���+0x60���жϣ��������жϵĶκ����ڲ���ϵͳ��
	setgate(idt+0x41,(int)int41h,2<<3,INTGATE32+0x60);
	setgate(idt+0x42,(int)int42h,2<<3,INTGATE32+0x60);
	setgate(idt+0x43,(int)int43h,2<<3,INTGATE32+0x60);
	
	io_loadidtr(LIMIT_IDT, ADR_IDT);						//����Ч�趨�������ڴ���ʼ��ַ����CPU�ڵ�IDTR�Ĵ���
	
}

void setseg(struct segment_descriptor *seg, unsigned int limit, int base, 			int ar)		//���ò����Խṹ��ĸ��������������ӳ�����дGDT��8�ֽ���Ϣ��
{	//ע����ڶκż�¼��Ļ�ַ//��ַ���ޣ��յ�ַ��//�Ǽ��ڸö��ϵ��������ڴ��еĻ�ַ��ʼ��ַ��//������
	if (limit > 0xfffff) 								//��������޳�����20λ����ͨ����µĶ�������ָ�������ֵ��
	{
		ar |= 0x8000;								//�趨��չ����Ȩ����λ�е�GD00�е�G=1��ʹ�øöεĶ����޵�λ����Ϊ4kb�Ӷ�ʹ�ö�������ָ�����4gb 
		limit /= 0x1000;							//��ʱ�����޵ĵ�λΪ4kb�ʳ���0x1000�õ���Ӳ�����һ�µĶ�������ֵ
	}
	seg->limit_low    = limit & 0xffff;						//ȡ�����޵ĵ�2�ֽ�
	seg->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);	//������������2�ֽں�ʹ�ø�2�ֽڵ���2�ֽ�λ�ò�ȡ���еĵ�4λ��������������1�ֽڲ�ȡ��1�ֽڵĸ���λ
	seg->base_low     = base & 0xffff;						//ȡ��ַ�ĵ�2�ֽ�			
	seg->base_mid     = (base >> 16) & 0xff;					//����ַ����2�ֽں�ȡ��1�ֽ�
	seg->base_high    = (base >> 24) & 0xff;					//����ַ����3�ֽں�ȡ��1�ֽ�
	seg->access_right = ar & 0xff;							//ȡ�����Եĵ�1�ֽڣ�����λ�Ѿ�������limit_high�У�
}

void setgate(struct gate_descriptor *gate, int offset,		 int selector,		 int ar)	//���ò����Խṹ��ĸ��������������ӳ�����дIDT��8�ֽ���Ϣ��
{	//ע������жϼ�¼��Ļ�ַ	//�жϴ���������ڵ�ַ//�ô������λ����һ��//������
	gate->offset_low   = offset & 0xffff;
	gate->offset_high  = (offset >> 16) & 0xffff;
	gate->selector     = selector;
	gate->dw_count     = (ar >> 8) & 0xff;
	gate->access_right = ar & 0xff;	
}

void initPIC(void)						//PICΪ������8���ж��źŵĿɱ���жϿ���������ʼ��PIC����ʼ���ж�����
{
	io_out8(PIC0_IMR,0xff);					//IMRΪ�жϼĴ������������ж�ʱ�����������ж����������ù����г����ж϶���������
	io_out8(PIC1_IMR,0xff);					//��1��ʾ���θ�λ�ж�
	
	io_out8(PIC0_ICW1,0x11);				//ICWΪ��ʼ���������ݣ�ICW1��ICW4��Ӳ������������Ϊ��ֵ
	io_out8(PIC0_ICW2,0x20);				//�趨�жϺŴ�0x20��ʼ�ֱ���IRQ���ж��źţ�0~8һһ��Ӧ
	io_out8(PIC0_ICW3,1<<2);				//��1������λ��Ϊ00000100����ʾ��PIC����PIC��IRQ2���ӣ�Ӳ�����������ֻ����ӣ�
	io_out8(PIC0_ICW4,0x01);
	
	io_out8(PIC1_ICW1,0x11);				//���ش���ģʽ
	io_out8(PIC1_ICW2,0x28);
	io_out8(PIC1_ICW3,2);					//��ʾ��PIC����PIC��IRQ2���ӣ�ֱ��дIRQ��ţ�
	io_out8(PIC1_ICW4,0x01);				//�޻�����ģʽ
	
	io_out8(PIC0_IMR,0xfb);					//����PIC0�г�IRQ2��������жϣ�����λ�����ö��ǵ���������룩����IRQ2Ϊ0��Ӧ��0xbf��Ȼ�����뵽����ʱ��Ҫ����
	io_out8(PIC1_IMR,0xff);					//����PIC1�����ж�
}

void initint(void)
{
	initPIC();	
	initGDTIDT();
	io_sti();
	initPIT();
	io_out8(PIC0_IMR, 0xf8); 			//���ż��̺ʹ�pic�ͼ�ʱ���жϣ����̽�IRQ1����pic��IRQ2����ʱ����IRQ0���趨PIC0��IMRΪ11111000��
	io_out8(PIC1_IMR, 0xef);  						//��������жϣ�����IRQ12���趨PIC1��IMRΪ11101111��	
}

void initPIT(void)						//��ʼ��PIT�Լ��ʱ�������趨IRQ0��
{
	io_out8(PIT_CTRL,0x34);					//����8254оƬ�ֲ���0x43�˿ڴ���0x34
	io_out8(PIT_CNT0,0x9c);					//�ж����ڵĵͰ�λ
	io_out8(PIT_CNT0,0x2e);					//�ж����ڵĸ߰�λ���ж�Ƶ��=cpu��Ƶ/�ж����ڣ�����10�������һ��ʱ���жϣ�
	timerctl->timerhead=0;					//��ʼ������ͷָ��0
	timerctl->counter=0;					//��ʼ����ʱ��
}
