;供应用程序调用的例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "appfunc.nas"]			;源文件名

	GLOBAL _getfifo
	
[SECTION .text]				;在目标文件中先加载这些再写程序

_getfifo:				;int getfifo();接受缓冲区输入
	mov	edx,5
	int	0x40
	ret