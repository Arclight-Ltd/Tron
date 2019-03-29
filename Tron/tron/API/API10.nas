;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _initmalloc
	
[SECTION .text]				;在目标文件中先加载这些再写程序
	
_initmalloc:				; void initmalloc(void);初始化应用程序的内存管理表
	push	ebx
	mov	edx,1
	mov	ebx,[cs:0x0020]		; malloc管理表的地址
	mov	eax,ebx			
	add	eax,32*1024		;内存空间起始地址=malloc管理表的地址+malloc管理表的大小
	mov	ecx,[cs:0x0000]		
	sub	ecx,eax			;内存空间大小=数据段大小-内存空间起始地址
	int	0x42
	pop	ebx
	ret