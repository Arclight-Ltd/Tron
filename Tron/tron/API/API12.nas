;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _free
	
[SECTION .text]				;在目标文件中先加载这些再写程序

_free:					; void free(char *addr, int size);释放获得的内存空间
	push	ebx
	mov	edx,3
	mov	ebx,[cs:0x0020]
	mov	eax,[esp+ 8]		; addr
	mov	ecx,[esp+12]		; size
	int	0x42
	pop	ebx
	ret
		