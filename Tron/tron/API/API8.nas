;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _squerwin
	
[SECTION .text]				;在目标文件中先加载这些再写程序
	
_squerwin:				; void squerwin(int x0, int x1, int y0,  int y1, int winsheet, int color);在创建的窗口中显示矩形
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