;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _malloc
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����

_malloc:				; char *malloc(int size);����ڴ�ռ�
	push	ebx
	mov	edx,2
	mov	ebx,[cs:0x0020]
	mov	ecx,[esp+8]		; size
	int	0x42
	pop	ebx
	ret