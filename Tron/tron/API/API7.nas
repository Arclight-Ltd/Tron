;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _putstrwin
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_putstrwin:				; void putstrwin(int winsheet, int x, int y, int color, int len, char *str);�ڴ����Ĵ�������ʾ�ַ���
	push	edi
	push	esi
	push	ebp
	push	ebx
	mov	edx,2
	mov	ebx,[esp+20]		; winsheet
	mov	esi,[esp+24]		; x
	mov	edi,[esp+28]		; y
	mov	eax,[esp+32]		; color
	mov	ecx,[esp+36]		; len
	mov	ebp,[esp+40]		; str
	int	0x41
	pop	ebx
	pop	ebp
	pop	esi
	pop	edi
	ret