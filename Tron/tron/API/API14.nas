;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _timeron
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����

_timeron:				; void timeron(unsigned int timer);������ʱ��
	push	ebx
	mov	edx,2
	mov	ebx,[esp+8]		; timer
	int	0x43
	pop	ebx
	ret
		