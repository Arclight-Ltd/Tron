;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _free
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����

_free:					; void free(char *addr, int size);�ͷŻ�õ��ڴ�ռ�
	push	ebx
	mov	edx,3
	mov	ebx,[cs:0x0020]
	mov	eax,[esp+ 8]		; addr
	mov	ecx,[esp+12]		; size
	int	0x42
	pop	ebx
	ret
		