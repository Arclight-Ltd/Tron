;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _putstr0
	
[SECTION .text]				;在目标文件中先加载这些再写程序
	
_putstr0:				;void putstr0(char*i)	在命令行窗口显示字符串
	push	ebx
	mov	edx,2			;调用二号功能
	mov	ebx,[esp+8]		;将字符串首地址（相对于该应用程序首地址而得出的地址）赋给ebx
	int	0x40			;调用显示单个字符的中断例程
	pop	ebx
	ret				;返回调用程序