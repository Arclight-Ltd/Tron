;�û��д���ж�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]			;�����´������ΪX486�Ժ��CPU����		
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "intfunc.nas"]			;Դ�ļ���
		
	GLOBAL _int0dh,_int0ch
	GLOBAL _int20h,_int21h,_int27h,_int2ch				;����ȫ�ֺ��� 
	GLOBAL _endapp,_int40h,_int41h,_int42h,_int43h
	
	EXTERN _int_0dh,_int_0ch
	EXTERN _int_20h,_int_21h,_int_27h,_int_2ch			;�������õĺ����ڱ��ļ�֮��
	EXTERN _int_40h,_int_41h,_int_42h,_int_43h
			
[SECTION .text]				;�����³�����ڴ���Σ�.obj�ļ���˵�ı�����

;*********************************************************�쳣�������ж�(0x00-0x1f)********************************************************

_int0dh:				;void int0dh(void);����һ�㱣���쳣ʱ�������жϣ�Ӧ�ó��������ϵͳ��д�����ݣ�
	sti				;���жϲ���Ҫ�����жϽ��в���
	push	es
	push	ds			;��Ȼ��ջ����16λ�Ĵ�������ջ��Ĭ��ռ��32λ�ռ�
	pushad	
	mov	eax,esp
	push	eax			;����ʱ��esp��Ϊʵ����ջ���ݸ�void int_0dh(void)
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;�������������жϳ���һ��
	
	call	_int_0dh
	cmp	eax,0			
	jne	_endapp			;�����Ϊ0˵������Ӧ�ó����������쳣����תִ�йر�Ӧ�ó���Ľ�������
	
	pop	eax			;��ͨ����ϵͳ���жϽ�������
	popad
	pop	ds
	pop	es
	add	esp,4			;int 0x0d�б��������		
	iretd

_int0ch:				;void int0ch(void);����ջ�쳣ʱ�������жϣ�Ӧ�ó��������ϵͳ��д�����ݣ�
	sti				;���жϲ���Ҫ�����жϽ��в���
	push	es
	push	ds
	pushad	
	mov	eax,esp
	push	eax	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;�������������жϳ���һ��
	
	call	_int_0ch
	cmp	eax,0			
	jne	_endapp			;�����Ϊ0˵������Ӧ�ó����������쳣����תִ�йر�Ӧ�ó���Ľ�������
	
	pop	eax			;��ͨ����ϵͳ���жϽ�������
	popad
	pop	ds
	pop	es
	add	esp,4			;�����쳣���ж��б��������		
	iretd	

;********************************************************IRQ�ж�(0x20-0x2f)****************************************************************

_int20h:				;void int20h(void);����20h��ʱ���ж�
	push 	es
	push	ds
	pushad				;��ͨ�üĴ�����ջ
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	
	call	_int_20h			;���ò����еĺ���
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32λ���жϷ���ָ��
	
_int21h:				;void int21h(void);����21h�����ж�
	push 	es
	push	ds
	pushad				;��ͨ�üĴ�����ջ
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	
	call	_int_21h			;���ò����еĺ���
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32λ���жϷ���ָ��
	
_int27h:				;void int27h(void);����27h�ж�
	push 	es
	push	ds
	pushad				;��ͨ�üĴ�����ջ
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	
	call	_int_27h			;���ò����еĺ���
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32λ���жϷ���ָ��
	
_int2ch:				;void int2ch(void);����2ch����ж�
	push 	es
	push	ds
	pushad				;��ͨ�üĴ�����ջ
	
	mov	eax,esp
	push	eax
	
	mov	ax,ss
	mov	ds,ax
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	
	call	_int_2ch			;���ò����еĺ���
	pop	eax
	popad
	pop 	ds
	pop	es
	iretd				;32λ���жϷ���ָ��

		
;********************************************��Ӧ�ó�����õ�ϵͳAPI(0x30-0xff)************************************************************
					;����APIǰss/ds��esp����CPU�Զ��л�������ϵͳ�����ݶ�

_endapp:				;void endapp(void);����Ӧ�ó���	��ʱeax=&(tss.esp0)
	mov	esp,[eax]		;esp=����ϵͳ->tss.esp0��ִ���ж�ʱss/ds���Զ��л�������ϵͳ�ĶΣ�
	mov	dword 	[eax+4],0	;����ϵͳ->tss.ss0=0  ��ִ��Ӧ�ó���ʱʹss0=0
	popad				;�����iofunction.nas��_io_startapp�ĵ�һ��pushad
	ret				;����������_io_startapp�ĳ���
	
_int40h:				;void int40h(void)	���ܣ��������д�����ʾ�ַ������Ӧ�ó���		������edx=���ܺ�
	sti				
	push	ds
	push	es
	pushad				;����Ϊһ���ж�Ҫִ�еĳ���
	
	pushad				;��Ϊvoid int_40h(void)�Ĳ�����ջ
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	call	_int_40h		;���ù��ܺ�ѡ���ӳ���_int_40h�ڲ���ϵͳ�Ķ��У���ʱ��ss�Ѿ�ָ���˲���ϵͳ��ջ�ʿ��Ե��ã�
	cmp	eax,0			;������ص��Ƿ�0˵�����쳣����ת������Ӧ�ó���Ϊ0˵������������ͨ��iretd��������
	jne	_endapp
	add	esp,32			;�൱��popad
	popad
	pop	es
	pop	ds
	iretd				;���ý����󷵻ص�Ӧ�ó��򣬴�ʱss/ds�ٴ��Զ��л���Ӧ�ó�������ݶ�
	
_int41h:				;void int40h(void)	���ܣ���Ӧ�ó��򴴽����ڵ�		������edx=���ܺ�
	sti				
	push	ds
	push	es
	pushad				;����Ϊһ���ж�Ҫִ�еĳ���
	
	pushad				;��Ϊvoid int_40h(void)�Ĳ�����ջ
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	call	_int_41h		;���ù��ܺ�ѡ���ӳ���_int_40h�ڲ���ϵͳ�Ķ��У���ʱ��ss�Ѿ�ָ���˲���ϵͳ��ջ�ʿ��Ե��ã�
	add	esp,32			;�൱��popad
	popad
	pop	es
	pop	ds
	iretd				;���ý����󷵻ص�Ӧ�ó��򣬴�ʱss/ds�ٴ��Զ��л���Ӧ�ó�������ݶ�
	
_int42h:				;void int40h(void)	���ܣ��ڴ����				������edx=���ܺ�
	sti				
	push	ds
	push	es
	pushad				;����Ϊһ���ж�Ҫִ�еĳ���
	
	pushad				;��Ϊvoid int_40h(void)�Ĳ�����ջ
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	call	_int_42h		;���ù��ܺ�ѡ���ӳ���_int_40h�ڲ���ϵͳ�Ķ��У���ʱ��ss�Ѿ�ָ���˲���ϵͳ��ջ�ʿ��Ե��ã�
	add	esp,32			;�൱��popad
	popad
	pop	es
	pop	ds
	iretd				;���ý����󷵻ص�Ӧ�ó��򣬴�ʱss/ds�ٴ��Զ��л���Ӧ�ó�������ݶ�
	
_int43h:				;void int40h(void)	���ܣ���ʱ��				������edx=���ܺ�
	sti				
	push	ds
	push	es
	pushad				;����Ϊһ���ж�Ҫִ�еĳ���
	
	pushad				;��Ϊvoid int_40h(void)�Ĳ�����ջ
	
	mov	ax,ss			
	mov	ds,ax		
	mov	es,ax			;ͳһDS��ES��SS��ֵ��Ϊ����c���Ժ�����׼��
	call	_int_43h		;���ù��ܺ�ѡ���ӳ���_int_40h�ڲ���ϵͳ�Ķ��У���ʱ��ss�Ѿ�ָ���˲���ϵͳ��ջ�ʿ��Ե��ã�
	add	esp,32			;�൱��popad
	popad
	pop	es
	pop	ds
	iretd				;���ý����󷵻ص�Ӧ�ó��򣬴�ʱss/ds�ٴ��Զ��л���Ӧ�ó�������ݶ�
	