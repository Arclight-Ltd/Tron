;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _putchar
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_putchar:				;void putchar(char i)������������������ַ�
	mov	al,[esp+4]
	mov	edx,1
	int	0x40
	ret
	