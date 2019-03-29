
# 默认动作

default :
	..\..\tools\make.exe run

# 文件生成规则

Tron.img : TRON\guide.bin TRON\Tron.sys APP1\app1.hrb Makefile
	..\..\tools\edimg.exe   imgin:..\..\tools\fdimg0at.tek \
		wbinimg src:TRON\guide.bin len:512 from:0 to:0 \
		copy from:TRON\Tron.sys to:@: \
		copy from:TRON\assembly.nas to:@: \
		copy from:TRON\console.obj to:@: \
		copy from:APP1\app1.hrb to:@: \
		imgout:Tron.img

# 命令

img :	
	..\..\tools\make.exe -r Tron.img

make:	
	..\..\tools\make.exe -C TRON 	make
	..\..\tools\make.exe -C APP1	make
	..\..\tools\make.exe  img
run :	
	..\..\tools\make.exe -C TRON 	make
	..\..\tools\make.exe -C APP1	make
	..\..\tools\make.exe  img
	copy Tron.img ..\..\tools\qemu\fdimage0.bin
	..\..\tools\make.exe -r -C ..\..\tools\qemu

del :
	..\..\tools\make.exe -C TRON	clean
	..\..\tools\make.exe -C APP1	clean
	-del Tron.img

	
