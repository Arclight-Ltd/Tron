;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _putstr0
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_putstr0:				;void putstr0(char*i)	�������д�����ʾ�ַ���
	push	ebx
	mov	edx,2			;���ö��Ź���
	mov	ebx,[esp+8]		;���ַ����׵�ַ������ڸ�Ӧ�ó����׵�ַ���ó��ĵ�ַ������ebx
	int	0x40			;������ʾ�����ַ����ж�����
	pop	ebx
	ret				;���ص��ó���