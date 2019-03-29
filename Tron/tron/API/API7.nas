;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _putstrwin
	
[SECTION .text]				;在目标文件中先加载这些再写程序
	
_putstrwin:				; void putstrwin(int winsheet, int x, int y, int color, int len, char *str);在创建的窗口中显示字符串
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