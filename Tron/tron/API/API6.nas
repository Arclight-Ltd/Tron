;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _window
	
[SECTION .text]				;在目标文件中先加载这些再写程序
	
_window:				;int window(char *buf, int xsize, int ysize, int color, char *title);创建窗口
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