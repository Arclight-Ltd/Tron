;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _initmalloc
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_initmalloc:				; void initmalloc(void);��ʼ��Ӧ�ó�����ڴ�����
	push	ebx
	mov	edx,1
	mov	ebx,[cs:0x0020]		; malloc�����ĵ�ַ
	mov	eax,ebx			
	add	eax,32*1024		;�ڴ�ռ���ʼ��ַ=malloc�����ĵ�ַ+malloc�����Ĵ�С
	mov	ecx,[cs:0x0000]		
	sub	ecx,eax			;�ڴ�ռ��С=���ݶδ�С-�ڴ�ռ���ʼ��ַ
	int	0x42
	pop	ebx
	ret