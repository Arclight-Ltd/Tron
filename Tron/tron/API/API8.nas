;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _squerwin
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_squerwin:				; void squerwin(int x0, int x1, int y0,  int y1, int winsheet, int color);�ڴ����Ĵ�������ʾ����
	push	edi
	push	esi
	push	ebp
	push	ebx
	mov	edx,3
	mov	eax,[esp+20]		; x0
	mov	esi,[esp+24]		; x1
	mov	ecx,[esp+28]		; y0
	mov	edi,[esp+32]		; y1
	mov	ebx,[esp+36]		; winsheet
	mov	ebp,[esp+40]		; color
	int	0x41
	pop	ebx
	pop	ebp
	pop	esi
	pop	edi
	ret	