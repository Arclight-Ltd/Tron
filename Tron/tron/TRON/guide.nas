;Tron启动区
; tab=8
	CYLS	equ	10			;宏定义（定义CYLS为常量10）
        org     0x7c00		            	;将以下程序装载到内存的0:7c00

                                   ;以下是标准fat12软盘的初始化（p24）

	jmp     entry                           ;执行主程序
	db	 0x90
	db   	"LC IPL  "		        ; 启动区名称（必须为8字节）（不够补空格）
	dw   	512			        ; 每个扇区的大小（必须为512字节）
	db   	1			        ; 簇的大小（必须为1个扇区）
	dw   	1			        ; fat起始位置（一般为第一个扇区）
	db      2			       	; fat的个数（必须为2）
	dw      224			      	;根目录的大小（一般为224项）
	dw      2880		               	; 磁盘的大小（必须是2880扇区）
	db      0xf0			        ; 磁盘种类（必须是0xf0）
	dw    	9			        ;fat的长度（必须是9扇区）
	dw   	18			        ; 1个磁道有几个扇区（必须是18）
	dw 	2			        ;磁头数（必须是2）
	dd	0			        ; 不使用分区，必须是0
	dd	2880		            	;重申磁盘大小
	db      0,0,0x29	            	; 固定，意义不明
	dd      0xffffffff	            	; （可能是）卷标号码
	db      "bubble     "	                ;磁盘名称（11字节）
	db	"fat12	 "	            	; 磁盘格式名称（8字节）
	resb    18				; 先空出18字节 

                                     ;主程序
entry:
	mov	ax,0		                ;寄存器初始化
	mov	ss,ax
	mov	sp,0x7c00                       ;栈顶指向0:7c00h
	mov	ds,ax
                                    ;以下程序读磁盘 
	mov	ax,0x0820
	mov	es,ax           						
	mov	dh,0	                        ;磁头0		
	mov	cl,2				;扇区2	
	mov	ch,0x00				;柱面0	
				
read:		
	mov	si,0                            ;由于软盘不稳定，允许5次读取出错
retry:	
	mov	ah,0x02		               	;ah=0x02 : 调用读入磁盘功能
	mov	al,1			        ;读取1个扇区
	mov	bx,0                            ;es:bx指向缓冲地址（读入数据在内存的装载起始地址）	
	mov	dl,0x00			        ;a驱动器
	int	0x13			        ;调用bios13h号中断例程
	jnc     next                        ;若没有进位说明没有错误，读取下一扇区
	inc	si
	cmp	si,5
	jnb	error
	mov	ah,0x00	
	mov	dl,0x00
	int	0x13
	jmp	retry                           ;若试错5次后仍然错误则跳转至错误提示程序
	
				;读取下一个扇区（磁头、柱面）
next:
	mov	ax,es
	add	ax,0x0020
	mov	es,ax				;使得es:bx指向下一个200H（扇区）的内存区
	inc	cl				;读取下一个扇区
	cmp	cl,18				
	jna	read				;如果读取<=18个扇区就跳转至read
	mov	cl,1				;cl=19时就初始化扇面	
	inc	dh				;从0磁头加到1磁头开始读取
	cmp	dh,2				
	jb	read				;若磁头为1则不改变柱面，继续读盘
	mov	dh,0				;若磁头加到2就重新赋值为1
	inc	ch				;读取下一个柱面
	cmp	ch,CYLS				;看是否到达了初设的预读柱面数
	je	fin				;若准备要读CYLS柱面0磁头1扇区就跳转至结束程序
	jmp	read
	
		              ;结束程序 	       
fin:
	mov	[0x0ff0],ch
	jmp	0xc200
 
                               ;出错处理程序
error:
	mov	si,msg                          ;si指向显示内容
putloop:
	mov	al,[si]
	inc     si	                        ;si指向下一个字节
	cmp	al,0                            ;若指向0说明显示结束
	je	fin                             ;显示结束跳转到结束程序
	mov	ah,0x0e		                ;调用显示文字功能
	mov	bx,15			        ;字符颜色
	int	0x10			        ;调用bios10h号例程
	jmp	putloop                         ;继续显示
msg:
	db	0x0a, 0x0a		        ;换行
	db	"load error"
	db	0x0a			
	db	0

	resb   	0x7dfe-$		        ;从此处到0x7dfe都装入0
    
	db	0x55, 0xaa                      ;表示这个扇区的开头是启动程序