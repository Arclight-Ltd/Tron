;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _window
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_window:				;int window(char *buf, int xsize, int ysize, int color, char *title);��������
	push	edi
	push	esi
	push	ebx
	mov	edx,1
	mov	ebx,[esp+16]		; buf
	mov	esi,[esp+20]		; xsize
	mov	edi,[esp+24]		; ysize
	mov	eax,[esp+28]		; color
	mov	ecx,[esp+32]		; title
	int	0x41
	pop	ebx
	pop	esi
	pop	edi
	ret