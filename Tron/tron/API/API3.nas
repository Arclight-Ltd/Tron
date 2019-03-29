;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _putstr1
	
[SECTION .text]				;在目标文件中先加载这些再写程序

_putstr1:				;void putstr0(char*i,int num)在命令行窗口显示字符串
	push	ebx
	push	ecx
	mov	ebx,[esp+12]		;将字符串首地址（相对于该应用程序首地址而得出的地址）赋给ebx
	mov	ecx,[esp+16]
	mov	edx,3			;调用三号功能
	int	0x40			;调用显示单个字符的中断例程
	pop	ecx
	pop	ebx
	ret				;返回调用程序