;用汇编写的中断例程

[FORMAT "WCOFF"]			;制作目标文件的模式（输出格式）	
[INSTRSET "i486p"]			;将以下代码解释为X486以后的CPU代码		
[BITS 32]				;制作32位模式用的机器语言
			
					;制作目标文件的信息
[FILE "intfunc.nas"]			;源文件名
		
	GLOBAL _int0dh,_int0ch
	GLOBAL _int20h,_int21h,_int27h,_int2ch				;申明全局函数 
	GLOBAL _endapp,_int40h,_int41h,_int42h,_int43h
	
	EXTERN _int_0dh,_int_0ch
	EXTERN _int_20h,_int_21h,_int_27h,_int_2ch			;声明调用的函数在本文件之外
	EXTERN _int_40h,_int_41h,_int_42h,_int_43h
			
[SECTION .text]				;将以下程序放在代码段（.obj文件中说文本区）

;*********************************************************异常产生的中断(0x00-0x1f)********************************************************

_int0dh:				;void int0dh(void);出现一般保护异常时产生的中断（应用程序向操作系统段写入数据）
	sti				;该中断不需要屏蔽中断进行操作
	push	es
	push	ds			;虽然入栈的是16位寄存器但在栈中默认占用32位空间
	pushad	
	mov	eax,esp
	push	eax			;将此时的esp作为实参入栈传递给void int_0dh(void)
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;以上与其他的中断程序一致
	
	call	_int_0dh
	cmp	eax,0			
	jne	_endapp			;如果不为0说明是由应用程序引发的异常，跳转执行关闭应用程序的结束程序
	
	pop	eax			;普通操作系统的中断结束程序
	popad
	pop	ds
	pop	es
	add	esp,4			;int 0x0d中必须有这句		
	iretd

_int0ch:				;void int0ch(void);出现栈异常时产生的中断（应用程序向操作系统段写入数据）
	sti				;该中断不需要屏蔽中断进行操作
	push	es
	push	ds
	pushad	
	mov	eax,esp
	push	eax	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;以上与其他的中断程序一致
	
	call	_int_0ch
	cmp	eax,0			
	jne	_endapp			;如果不为0说明是由应用程序引发的异常，跳转执行关闭应用程序的结束程序
	
	pop	eax			;普通操作系统的中断结束程序
	popad
	pop	ds
	pop	es
	add	esp,4			;处理异常的中断中必须有这句		
	iretd	

;********************************************************IRQ中断(0x20-0x2f)****************************************************************

_int20h:				;void int20h(void);调用20h定时器中断
	push 	es
	push	ds
	pushad				;将通用寄存器入栈
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	
	call	_int_20h			;调用参数中的函数
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32位的中断返回指令
	
_int21h:				;void int21h(void);调用21h键盘中断
	push 	es
	push	ds
	pushad				;将通用寄存器入栈
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	
	call	_int_21h			;调用参数中的函数
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32位的中断返回指令
	
_int27h:				;void int27h(void);调用27h中断
	push 	es
	push	ds
	pushad				;将通用寄存器入栈
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	
	call	_int_27h			;调用参数中的函数
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32位的中断返回指令
	
_int2ch:				;void int2ch(void);调用2ch鼠标中断
	push 	es
	push	ds
	pushad				;将通用寄存器入栈
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	
	call	_int_2ch			;调用参数中的函数
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32位的中断返回指令

		
;********************************************供应用程序调用的系统API(0x30-0xff)************************************************************
					;调用API前ss/ds和esp会由CPU自动切换到操作系统的数据段

_endapp:				;void endapp(void);结束应用程序	此时eax=&(tss.esp0)
	mov	esp,[eax]		;esp=操作系统->tss.esp0（执行中断时ss/ds都自动切换到操作系统的段）
	mov	dword 	[eax+4],0	;操作系统->tss.ss0=0  不执行应用程序时使ss0=0
	popad				;这里接iofunction.nas中_io_startapp的第一个pushad
	ret				;跳回至调用_io_startapp的程序
	
_int40h:				;void int40h(void)	功能：在命令行窗口显示字符或结束应用程序		参数：edx=功能号
	sti				
	push	ds
	push	es
	pushad				;以上为一般中断要执行的程序
	
	pushad				;作为void int_40h(void)的参数入栈
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	call	_int_40h		;调用功能号选择子程序（_int_40h在操作系统的段中，此时的ss已经指向了操作系统的栈故可以调用）
	cmp	eax,0			;如果返回的是非0说明是异常，跳转到结束应用程序，为0说明是正常程序，通过iretd正常返回
	jne	_endapp
	add	esp,32			;相当于popad
	popad
	pop	es
	pop	ds
	iretd				;调用结束后返回到应用程序，此时ss/ds再次自动切换到应用程序的数据段
	
_int41h:				;void int40h(void)	功能：在应用程序创建窗口等		参数：edx=功能号
	sti				
	push	ds
	push	es
	pushad				;以上为一般中断要执行的程序
	
	pushad				;作为void int_40h(void)的参数入栈
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	call	_int_41h		;调用功能号选择子程序（_int_40h在操作系统的段中，此时的ss已经指向了操作系统的栈故可以调用）
	add	esp,32			;相当于popad
	popad
	pop	es
	pop	ds
	iretd				;调用结束后返回到应用程序，此时ss/ds再次自动切换到应用程序的数据段
	
_int42h:				;void int40h(void)	功能：内存管理				参数：edx=功能号
	sti				
	push	ds
	push	es
	pushad				;以上为一般中断要执行的程序
	
	pushad				;作为void int_40h(void)的参数入栈
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	call	_int_42h		;调用功能号选择子程序（_int_40h在操作系统的段中，此时的ss已经指向了操作系统的栈故可以调用）
	add	esp,32			;相当于popad
	popad
	pop	es
	pop	ds
	iretd				;调用结束后返回到应用程序，此时ss/ds再次自动切换到应用程序的数据段
	
_int43h:				;void int40h(void)	功能：定时器				参数：edx=功能号
	sti				
	push	ds
	push	es
	pushad				;以上为一般中断要执行的程序
	
	pushad				;作为void int_40h(void)的参数入栈
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;统一DS、ES、SS的值，为调用c语言函数做准备
	call	_int_43h		;调用功能号选择子程序（_int_40h在操作系统的段中，此时的ss已经指向了操作系统的栈故可以调用）
	add	esp,32			;相当于popad
	popad
	pop	es
	pop	ds
	iretd				;调用结束后返回到应用程序，此时ss/ds再次自动切换到应用程序的数据段
	