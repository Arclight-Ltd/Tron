;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _putstr1
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����

_putstr1:				;void putstr0(char*i,int num)�������д�����ʾ�ַ���
	push	ebx
	push	ecx
	mov	ebx,[esp+12]		;���ַ����׵�ַ������ڸ�Ӧ�ó����׵�ַ���ó��ĵ�ַ������ebx
	mov	ecx,[esp+16]
	mov	edx,3			;�������Ź���
	int	0x40			;������ʾ�����ַ����ж�����
	pop	ecx
	pop	ebx
	ret				;���ص��ó���