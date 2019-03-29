
OBJS_API = API1.obj API2.obj API3.obj API4.obj API5.obj API6.obj API7.obj API8.obj API9.obj API10.obj API11.obj API12.obj \
	   API13.obj API14.obj API15.obj alloca.obj

GOLIB    = ..\..\..\tools\golib00.exe 

# 默认动作

default :
	..\..\..\tools\make.exe lib

# 文件生成规则

API.lib : Makefile $(OBJS_API)
	$(GOLIB) $(OBJS_API) out:API.lib

%.obj : %.nas Makefile
	..\..\..\tools\nask.exe $*.nas $*.obj $*.lst

# 命令

del :
	-del *.obj
	-del *.lst
	-del API.lib

lib : 	..\..\..\tools\make.exe API.lib	
