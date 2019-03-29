;跳入保护模式代码如下assembly.nas（1）准备GDT（2）用LGDT加载gdtr（3）打开A20（4）设置CR0的PE位（5）跳转进入保护模式

BOTPAK	EQU		0x00280000		;接收harimain.hrb首地址
DSKCAC	EQU		0x00100000		;接受启动区的首地址
DSKCAC0	EQU		0x00008000+512		;要被传送的除启动区外的剩余数据的首地址

CYLS	EQU		0x0ff0			;设定开始储存区

LEDS	EQU		0x0ff1			;键盘led灯指示状态
VMODE	EQU		0x0ff2			;颜色位数的信息
XNUMBER	EQU		0x0ff4			;分辨率中X的个数
YNUMBER	EQU		0x0ff6			;分辨率中y的个数
VRAM	EQU		0x0ff8			;当前模式下图像缓冲区的首地址

		ORG		0xc200	 ;让启动区加载后从这里开始运行，认为它们在一个段中所以能直接跳转过来		



		MOV		BX,0x4105			;进入VBE1280X1024 的8位彩色显卡模式
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8			;保存当前画面模式信息
		MOV		WORD [XNUMBER],1024
		MOV		WORD [YNUMBER],768
		MOV		DWORD [VRAM],0xe0000000
		MOV		AH,0x02
		INT		0x16 				; keyboard BIOS
		MOV		[LEDS],AL			;储存键盘led灯指示状态

;在模式转换与PIC初始化前关闭一切中断

		MOV		AL,0xff                 
      		OUT		0x21,AL			;禁止主PIC的中断
		NOP					;让CPU休息一个时钟周期（不能有两个连续的OUT指令，不然有些机种无法正常运行）
		OUT		0xa1,AL			;禁止从PIC的中断

		CLI		                        ;禁止CPU级别的中断（禁止PIC工作要在CLI之前，否则有时会挂起）

;为了让CPU能访问1MB以上的内存，设定A20 GATE

		CALL	waitkbdout                      ;等待键盘电路准备好
		MOV		AL,0xd1
		OUT		0x64,AL			;向键盘命令端口输出命令，选择模式（选中键盘控制电路的附属端口）
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL			;向键盘数据端口输出数据（使内存1MB以上的部分变成可用状态）
		CALL	waitkbdout			;等待命令被切实执行

;开始切换到保护模式

[INSTRSET "i486p"]					;以下指令被解释为X486的汇编指令（为了能使用LGDT,EAX,CR0等关键词）

		LGDT	[GDTR0]			        ;加载临时的GDT表首地址到GDTR寄存器
		MOV		EAX,CR0			;control register 0 ，只有操作系统才能操作它
		AND		EAX,0x7fffffff		;设第31位为0（为了禁止颁）
		OR		EAX,0x00000001		;设第0位为1（为了切换到保护模式）
		MOV		CR0,EAX                 ;设置CR0寄存器PE标志位（进入到不用颁的保护模式）
		JMP		pipelineflush           ;设置标志位之后必须马上JMP		
pipelineflush:                                          ;从此寻址方式变了，段寄存器的意思也变了，要重新解释段寄存器
		MOV		AX,1*8                  ;段寄存器值的设置，用GDT中那个段（GDT+1段）			
		MOV		DS,AX			;除cs外的所有段寄存器初始化为0x0008
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

;启动时所有数据都在前1MB中，进入保护模式后释放了内存，便可以把这些数据放到1MB之后的内存中去，前1MB留给BIOS和VRAM等（自制系统内存分布图P158）
		
;复制harimain.hrb

		MOV		ESI,harimain	        ;将harimain.hrb生成的目标代码移动到0x00280000（复制bootpack）（源地址）
		MOV		EDI,BOTPAK		;（目的地址）
		MOV		ECX,512*1024/4		;（传送数据大小）比harimain.hrb大,多赋予一些内存（512K）
		CALL	memcpy				;调用内存复制子程序

;复制启动区

		MOV		ESI,0x7c00              ;将启动扇区复制到1MB以后的内存		
		MOV		EDI,DSKCAC		
		MOV		ECX,512/4
		CALL	memcpy
		
;复制所有剩下的数据

		MOV		ESI,DSKCAC0	     ;将0x00008200的磁盘数据（除启动区外的其他数据的首地址）复制到0x00100200（接着上一次复制的目的地址）
		MOV		EDI,DSKCAC+512
		MOV		ECX,0 			;清空ECX
		MOV		CL,BYTE [CYLS]		;
		IMUL	        ECX,512*18*2/4		;（整数乘法运算）从柱面数变为字节数/4
		SUB		ECX,512/4		;减去IPL所占空间（这样0x00100000开始的部分就和软盘从0开始的内容一致了）	
		CALL	memcpy

;调用harimain.hrb的初始化操作，解析harimain.hrb的header并传入参数  
							
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]		;harimain.hrb后的第16号地址--所存的值为0x11a8（harimain.hrb文件数据部分大小）
		ADD		ECX,3			;ECX+=3
		SHR		ECX,2			;向右位移2位，相当于ECX/=4
		JZ		skip			;没有要转送的东西时（ECX=0）跳过转送阶段
		MOV		ESI,[EBX+20]		;转送源（harimain.hrb后的第20号地址--所存的值为0x10c8）（数据部分起始地址）
		ADD		ESI,EBX			;相对于harimain.hrb开头的0x10c8地址处
		MOV		EDI,[EBX+12]		;转送目的（harimain.hrb后的第12号地址--所存的值为0x00310000）（esp起始地址）
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]		;栈顶初始值
		JMP		DWORD 2*8:0x0000001b    ;将2*8代入CS，跳转到CS:0x1b（跳到指定段中，跳过可执行文件头p421从而开始执行harimain.hrb）
					;（跳到相对于第二个段(harimain.hrb)的0x1b）此模式中的cs被解释为第二个段（相当于cs的值为第二个段的基地址）

;等待键盘控制电路响应的子程序					
					
waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		 AL,0x60		;空读，清空数据接收缓冲区中的垃圾数据（若有数据则顺便读出来）
		JNZ		waitkbdout		;如果与运算结果不为0则继续等待响应
		RET

;内存复制子程序
		
memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			
		RET

;临时设计的GDT表

		ALIGNB	16					;一直添加DBO直到地址能被16整除
GDT0:                                                           
		RESB	8					;0号段，空区域，不能定义段
		DW		0xffff,0x0000,0x9200,0x00cf	;1号段，可读写的段，32bit
		DW		0xffff,0x0000,0x9a28,0x0047	;2号段，可执行的段，32bit（用于存放harimain.hrb）

		DW		0
		
;临时设计的GDT选择子

GDTR0:                                            		;相当于LGDT               
		DW		8*3-1
		DD		GDT0				;在GDT0中写入16位的段上限和32位的段基址

		ALIGNB	16					;一直添加DBO直到地址能被16整除
harimain:							;紧接着harimain.hrb的开头部分（由编译器实现），执行完assembly.nas后执行harimain.hrb