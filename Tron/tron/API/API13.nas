;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _timeralloc
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����

_timeralloc:				; int timeralloc(unsigned int timeout,char data);������ʱ��
	push	ebx
	push	ecx
	mov	edx,1
	mov	ebx,[esp+12]		; timeout
	mov	ecx,[esp+16]		; data
	int	0x43
	pop	ecx
	pop	ebx
	ret
		