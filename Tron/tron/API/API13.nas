;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _timeralloc
	
[SECTION .text]				;在目标文件中先加载这些再写程序

_timeralloc:				; int timeralloc(unsigned int timeout,char data);创建计时器
	push	ebx
	push	ecx
	mov	edx,1
	mov	ebx,[esp+12]		; timeout
	mov	ecx,[esp+16]		; data
	int	0x43
	pop	ecx
	pop	ebx
	ret
		