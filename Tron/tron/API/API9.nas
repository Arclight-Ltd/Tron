;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _closewin
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_closewin:				;void closewin(int win);�رմ���
	push 	ebx
	mov	edx,4
	mov	ebx,[esp+8]
	int	0x41
	pop	ebx
	ret