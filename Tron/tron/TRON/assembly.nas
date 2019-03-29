;���뱣��ģʽ��������assembly.nas��1��׼��GDT��2����LGDT����gdtr��3����A20��4������CR0��PEλ��5����ת���뱣��ģʽ

BOTPAK	EQU		0x00280000		;����harimain.hrb�׵�ַ
DSKCAC	EQU		0x00100000		;�������������׵�ַ
DSKCAC0	EQU		0x00008000+512		;Ҫ�����͵ĳ����������ʣ�����ݵ��׵�ַ

CYLS	EQU		0x0ff0			;�趨��ʼ������

LEDS	EQU		0x0ff1			;����led��ָʾ״̬
VMODE	EQU		0x0ff2			;��ɫλ������Ϣ
XNUMBER	EQU		0x0ff4			;�ֱ�����X�ĸ���
YNUMBER	EQU		0x0ff6			;�ֱ�����y�ĸ���
VRAM	EQU		0x0ff8			;��ǰģʽ��ͼ�񻺳������׵�ַ

		ORG		0xc200	 ;�����������غ�����￪ʼ���У���Ϊ������һ������������ֱ����ת����		



		MOV		BX,0x4105			;����VBE1280X1024 ��8λ��ɫ�Կ�ģʽ
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8			;���浱ǰ����ģʽ��Ϣ
		MOV		WORD [XNUMBER],1024
		MOV		WORD [YNUMBER],768
		MOV		DWORD [VRAM],0xe0000000
		MOV		AH,0x02
		INT		0x16 				; keyboard BIOS
		MOV		[LEDS],AL			;�������led��ָʾ״̬

;��ģʽת����PIC��ʼ��ǰ�ر�һ���ж�

		MOV		AL,0xff                 
      		OUT		0x21,AL			;��ֹ��PIC���ж�
		NOP					;��CPU��Ϣһ��ʱ�����ڣ�����������������OUTָ���Ȼ��Щ�����޷��������У�
		OUT		0xa1,AL			;��ֹ��PIC���ж�

		CLI		                        ;��ֹCPU������жϣ���ֹPIC����Ҫ��CLI֮ǰ��������ʱ�����

;Ϊ����CPU�ܷ���1MB���ϵ��ڴ棬�趨A20 GATE

		CALL	waitkbdout                      ;�ȴ����̵�·׼����
		MOV		AL,0xd1
		OUT		0x64,AL			;���������˿�������ѡ��ģʽ��ѡ�м��̿��Ƶ�·�ĸ����˿ڣ�
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL			;��������ݶ˿�������ݣ�ʹ�ڴ�1MB���ϵĲ��ֱ�ɿ���״̬��
		CALL	waitkbdout			;�ȴ������ʵִ��

;��ʼ�л�������ģʽ

[INSTRSET "i486p"]					;����ָ�����ΪX486�Ļ��ָ�Ϊ����ʹ��LGDT,EAX,CR0�ȹؼ��ʣ�

		LGDT	[GDTR0]			        ;������ʱ��GDT���׵�ַ��GDTR�Ĵ���
		MOV		EAX,CR0			;control register 0 ��ֻ�в���ϵͳ���ܲ�����
		AND		EAX,0x7fffffff		;���31λΪ0��Ϊ�˽�ֹ�䣩
		OR		EAX,0x00000001		;���0λΪ1��Ϊ���л�������ģʽ��
		MOV		CR0,EAX                 ;����CR0�Ĵ���PE��־λ�����뵽���ð�ı���ģʽ��
		JMP		pipelineflush           ;���ñ�־λ֮���������JMP		
pipelineflush:                                          ;�Ӵ�Ѱַ��ʽ���ˣ��μĴ�������˼Ҳ���ˣ�Ҫ���½��ͶμĴ���
		MOV		AX,1*8                  ;�μĴ���ֵ�����ã���GDT���Ǹ��Σ�GDT+1�Σ�			
		MOV		DS,AX			;��cs������жμĴ�����ʼ��Ϊ0x0008
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

;����ʱ�������ݶ���ǰ1MB�У����뱣��ģʽ���ͷ����ڴ棬����԰���Щ���ݷŵ�1MB֮����ڴ���ȥ��ǰ1MB����BIOS��VRAM�ȣ�����ϵͳ�ڴ�ֲ�ͼP158��
		
;����harimain.hrb

		MOV		ESI,harimain	        ;��harimain.hrb���ɵ�Ŀ������ƶ���0x00280000������bootpack����Դ��ַ��
		MOV		EDI,BOTPAK		;��Ŀ�ĵ�ַ��
		MOV		ECX,512*1024/4		;���������ݴ�С����harimain.hrb��,�ำ��һЩ�ڴ棨512K��
		CALL	memcpy				;�����ڴ渴���ӳ���

;����������

		MOV		ESI,0x7c00              ;�������������Ƶ�1MB�Ժ���ڴ�		
		MOV		EDI,DSKCAC		
		MOV		ECX,512/4
		CALL	memcpy
		
;��������ʣ�µ�����

		MOV		ESI,DSKCAC0	     ;��0x00008200�Ĵ������ݣ�������������������ݵ��׵�ַ�����Ƶ�0x00100200��������һ�θ��Ƶ�Ŀ�ĵ�ַ��
		MOV		EDI,DSKCAC+512
		MOV		ECX,0 			;���ECX
		MOV		CL,BYTE [CYLS]		;
		IMUL	        ECX,512*18*2/4		;�������˷����㣩����������Ϊ�ֽ���/4
		SUB		ECX,512/4		;��ȥIPL��ռ�ռ䣨����0x00100000��ʼ�Ĳ��־ͺ����̴�0��ʼ������һ���ˣ�	
		CALL	memcpy

;����harimain.hrb�ĳ�ʼ������������harimain.hrb��header���������  
							
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]		;harimain.hrb��ĵ�16�ŵ�ַ--�����ֵΪ0x11a8��harimain.hrb�ļ����ݲ��ִ�С��
		ADD		ECX,3			;ECX+=3
		SHR		ECX,2			;����λ��2λ���൱��ECX/=4
		JZ		skip			;û��Ҫת�͵Ķ���ʱ��ECX=0������ת�ͽ׶�
		MOV		ESI,[EBX+20]		;ת��Դ��harimain.hrb��ĵ�20�ŵ�ַ--�����ֵΪ0x10c8�������ݲ�����ʼ��ַ��
		ADD		ESI,EBX			;�����harimain.hrb��ͷ��0x10c8��ַ��
		MOV		EDI,[EBX+12]		;ת��Ŀ�ģ�harimain.hrb��ĵ�12�ŵ�ַ--�����ֵΪ0x00310000����esp��ʼ��ַ��
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]		;ջ����ʼֵ
		JMP		DWORD 2*8:0x0000001b    ;��2*8����CS����ת��CS:0x1b������ָ�����У�������ִ���ļ�ͷp421�Ӷ���ʼִ��harimain.hrb��
					;����������ڵڶ�����(harimain.hrb)��0x1b����ģʽ�е�cs������Ϊ�ڶ����Σ��൱��cs��ֵΪ�ڶ����εĻ���ַ��

;�ȴ����̿��Ƶ�·��Ӧ���ӳ���					
					
waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		 AL,0x60		;�ն���������ݽ��ջ������е��������ݣ�����������˳���������
		JNZ		waitkbdout		;�������������Ϊ0������ȴ���Ӧ
		RET

;�ڴ渴���ӳ���
		
memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			
		RET

;��ʱ��Ƶ�GDT��

		ALIGNB	16					;һֱ���DBOֱ����ַ�ܱ�16����
GDT0:                                                           
		RESB	8					;0�ŶΣ������򣬲��ܶ����
		DW		0xffff,0x0000,0x9200,0x00cf	;1�ŶΣ��ɶ�д�ĶΣ�32bit
		DW		0xffff,0x0000,0x9a28,0x0047	;2�ŶΣ���ִ�еĶΣ�32bit�����ڴ��harimain.hrb��

		DW		0
		
;��ʱ��Ƶ�GDTѡ����

GDTR0:                                            		;�൱��LGDT               
		DW		8*3-1
		DD		GDT0				;��GDT0��д��16λ�Ķ����޺�32λ�Ķλ�ַ

		ALIGNB	16					;һֱ���DBOֱ����ַ�ܱ�16����
harimain:							;������harimain.hrb�Ŀ�ͷ���֣��ɱ�����ʵ�֣���ִ����assembly.nas��ִ��harimain.hrb