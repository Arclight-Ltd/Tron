;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _timeroff
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����

_timeroff:				; void timeroff(unsigned int timer);�ͷż�ʱ��
	push	ebx
	mov	edx,3
	mov	ebx,[esp+8]		; timer
	int	0x43
	pop	ebx
	ret
		