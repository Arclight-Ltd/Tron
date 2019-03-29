
OBJS_TRON =harimain.obj iofunction.obj hankaku.obj display.obj init_int.obj int.obj portbuf.obj multask.obj\
		mouse.obj intfunc.obj memory.obj sheet.obj timer.obj console.obj cmd.obj loadfile.obj

# 默认动作

default :
	..\..\..\tools\make.exe make run

# 文件生成规则

%.bin : %.nas Makefile
	..\..\..\tools\nask.exe $*.nas $*.bin $*.lst
	
hankaku.bin : hankaku.txt Makefile
	..\..\..\tools\makefont.exe hankaku.txt hankaku.bin

iofunction.obj : iofunction.nas Makefile
	..\..\..\tools\nask.exe iofunction.nas iofunction.obj iofunction.lst
	
hankaku.obj : hankaku.bin Makefile
	..\..\..\tools\bin2obj.exe hankaku.bin hankaku.obj _hankaku

%.gas : %.c Makefile
	..\..\..\tools\cc1.exe -I ..\..\..\tools\haribote -Os -Wall -quiet -o $*.gas $*.c

%.nas : %.gas Makefile
	..\..\..\tools\gas2nask.exe -a $*.gas $*.nas

%.obj : %.nas Makefile
	..\..\..\tools\nask.exe $*.nas $*.obj $*.lst

harimain.bim :  Makefile  $(OBJS_TRON)
	..\..\..\tools\obj2bim.exe @..\..\..\tools\haribote\haribote.rul out:harimain.bim stack:3136k map:harimain.map $(OBJS_TRON)
			
# 3MB+64KB=3136KB

harimain.hrb : harimain.bim Makefile
	..\..\..\tools\bim2hrb.exe harimain.bim harimain.hrb 0	
	
Tron.sys : assembly.bin harimain.hrb Makefile
	copy /B assembly.bin+harimain.hrb Tron.sys

Tron.img : guide.bin Tron.sys Makefile
	..\..\..\tools\edimg.exe   imgin:..\..\..\tools\fdimg0at.tek \
		wbinimg src:guide.bin len:512 from:0 to:0 \
		copy from:Tron.sys to:@: \
		copy from:assembly.nas to:@: \
		copy from:console.obj to:@: \
		imgout:Tron.img

# 命令

run :
	..\..\..\tools\make.exe  Tron.img
	copy Tron.img ..\..\..\tools\qemu\fdimage0.bin
	..\..\..\tools\make.exe -r -C ..\..\..\tools\qemu
	
make :	..\..\..\tools\make.exe guide.bin
	..\..\..\tools\make.exe Tron.sys

clean :
	-del *.bin
	-del *.lst
	-del *.obj
	-del *.map
	-del *.bim
	-del *.hrb
	-del Tron.sys
del:	
	..\..\..\tools\make.exe clean
	-del Tron.img
