;用汇编写的供c语言调用的函数

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]					
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "iofunction.nas"]			;源文件名

	GLOBAL _io_hlt, _io_cli, _io_sti, _io_stihlt			;申明全局函数 
	GLOBAL _io_in8, _io_in16, _io_in32
	GLOBAL _io_out8, _io_out16, _io_out32
	GLOBAL _io_popeflag, _io_pusheflag
	GLOBAL _io_shl
	GLOBAL _io_loadgdtr, _io_loadidtr
	GLOBAL _io_loadcr0,_io_storecr0
	GLOBAL _io_count,_io_loadtr,_io_jmpfar,_io_callfar,_io_startapp
	
				;以下为自己编写的函数 
[SECTION .text]				;在目标文件中先加载这些再写程序

								
_io_hlt:				;void io_hlt(void);待机函数
	hlt
	ret
	
_io_cli:				;void io_cli(void);中断标志置0，屏蔽中断
	cli
	ret
	
_io_sti:				;void io_sti(void);中断标志置1，打开中断
	sti
	ret
	
_io_stihlt:				;void io_stihlt(void);打开中断后待机
	sti
	hlt
	ret
	
_io_in8:				;char io_in8(int port);从port端口号读入并返回8位的数据
	mov	edx,[esp+4]		;esp+4为c函数第一个参数的首地址
	mov	eax,0
	in	al,dx
	ret 

_io_in16:				;short io_in16(int port);从port端口号读入并返回16位的数据
	mov	edx,[esp+4]
	mov	eax,0
	in	ax,dx
	ret

_io_in32:				;int io_in32(int port);从port端口号读入并返回32位的数据
	mov	edx,[esp+4]
	in	eax,dx
	ret

_io_out8:				;void io_out8(int port,int data);给port端口号输出8位的数据（data参数）
	mov	edx,[esp+4]
	mov	eax,[esp+8]
	out	dx,al
	ret

_io_out16:				;void io_out16(int port,int data);给port端口号输出16位的数据（data参数）
	mov	edx,[esp+4]
	mov	eax,[esp+8]
	out	dx,ax
	ret

_io_out32:				;void io_out32(int port,int data);给port端口号输出32位的数据（data参数）
	mov	edx,[esp+4]
	mov	eax,[esp+8]
	out	dx,eax
	ret

_io_popeflag:				;int io_popeflag(void);返回当前标志寄存器的状态
	pushfd		
	pop	eax
	ret

_io_pusheflag:				;void io_pusheflag(int eflag);将数据（eflag参数）送入标志寄存器
	mov	eax,[esp+4]
	push	eax
	popfd
	ret
	
_io_shl:				;int io_shl(int byte);将byte按位左移一位
	mov	eax,[esp+4]
	shl	eax,1
	ret
	                                                                                              
_io_loadgdtr:				; void io_loadgdtr(int limit, int addr);将有效设定个数和内存起始地址放在cpu内的gdtr寄存器
	mov	ax,[esp+4]		;将limit（上限）的低四位赋予ax
	mov	[esp+6],ax		;将低四位转换到内存[esp+6]处，此时内存中内容见P113
	lgdt	[esp+6]			;将内存[esp+6]后的6字节内容读到GDTR寄存器
	ret

_io_loadidtr:				; void io_loadidtr(int limit, int addr);将有效设定个数和内存起始地址放在cpu内的idtr寄存器
	mov	ax,[esp+4]		; 将limit（上限）的低四位赋予ax
	mov	[esp+6],ax		; 内存内容（[esp+6]开始算）为[ff 07 00 f8 26 00]
	lidt	[esp+6]
	ret
	
_io_loadcr0:				;int loadcr0(void)  
	mov 	eax,cr0
	ret
	
_io_storecr0:				;void storecr0(int cr0)
	mov 	eax,[esp+4]
	mov	cr0,eax
	ret

_io_count:				;unsigned int count(unsigned int start,unsigned int end)
	push 	esi
	push 	edi
	push	edx
	push	ebx
	mov 	esi,0x55aa55aa				;作为试写的数据来检测内存是否可用
	mov 	edi,0xaa55aa55
	mov	eax,[esp+16+4]				;将start赋给eax[当前栈顶地址+4*4byte的入栈保存的寄存器+调用函数时保存了ip寄存器]
	text:		
		mov	ebx,eax				;eax指向4kb内存的首地址，ebx指向真正要检查的4kb内存的尾地址
		add	ebx,0xffc			;只检测eax为首地址的4kb内存的最后4个字节以提高执行速度
		mov	edx,[ebx]			;将最后四个字节的数据备份在edx中			
		mov	[ebx],esi			;将测试数据写入内存
		not	dword [ebx]			;将内存中的数据取反（检测常用取反）
		cmp	[ebx],edi			;若取反后不是预期结果说明该内存不可用，跳转到结束程序
		jne	notmemory
		not	dword [ebx]			;若可用则再次取反继续确认
		cmp	[ebx],esi
		jne	notmemory
		mov	[ebx],edx			;恢复内存原来的值。若两次都为预期结果说明该段内存可以使用
		add	eax,0x1000			;将eax加4kb使得eax指向下一个4kb内存的首地址
		cmp	eax,[esp+16+8]			;检查eax是否超出所给的内存检查最大地址
		jbe	text				;跳转继续检查
	notmemory:
		mov 	[ebx],edx			;恢复内存原来的值
		pop 	ebx
		pop	edx
		pop	edi
		pop	esi				;因为检查的是4kb内存的最后4字节，若最后4字节有用则返回的是下个4kb内存的首地址
		ret					;若最后4字节不能用则返回当前4kb内存的首地址（该地址前的内存已经保证能用了）（忽略中间的误差）

_io_loadtr:						;void io_loadtr(int tr)	
	ltr	[esp+4]					;只改变TR寄存器（任务寄存器）的值
	ret
	
_io_jmpfar:						;void io_jmpfar(int eip,int cs)	
	jmp	far	[esp+4]				;用作任务切换的jmp指令，在返回该任务时会从jmp指令之后恢复执行
	ret
	
_io_callfar:						;void io_callfar(int eip,int cs)	
	call	far	[esp+4]				;call指令以便调用结束时返回原程序
	ret	
	
_io_startapp:		; void io_startapp(int eip, int cs, int esp, int ds, int *tss_esp0);
	pushad					;保存所有寄存器的值
	mov	eax,[esp+36]			;eax=应用程序的eip（如果直接修改eip等的话会导致混乱）
	mov	ecx,[esp+40]			;ecx=应用程序的cs
	mov	edx,[esp+44]			;edx=应用程序的esp
	mov	ebx,[esp+48]			;ebx=应用程序的ds/ss
	mov	ebp,[esp+52]			;ebp=当前任务（操作系统）的tss中的esp0的地址
	mov	[ebp  ],esp			;当前任务->tss.esp0=esp（当前的栈顶指向保存所有寄存器后（pushad指令后））
	mov	[ebp+4],ss			;当前任务->tss.ss0=ss（将操作系统的栈的地址保存到tss中）（通知CPU这是操作系统数据段？）
	mov	es,bx				;使应用程序的es=ds（应用程序的其他段都指向其数据段）
	mov	ds,bx
	mov	fs,bx
	mov	gs,bx
						;X86不允许操作系统call或jmp应用程序
	or	ecx,3				;为用retf跳转到应用程序做准备
	or	ebx,3		
	
	push	ebx				;应用程序的ds/ss入栈
	push	edx				;应用程序的esp入栈
	push	ecx				;应用程序的cs入栈
	push	eax				;应用程序的eip入栈
	retf					;用栈中的cs:eip修改当前的cs:eip的值，实现向应用程序跳转（第一次跳转到应用程序后CPU会备份当前数据段并自动用栈中数据设置ss、ds、esp）
		