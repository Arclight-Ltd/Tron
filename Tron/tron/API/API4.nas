;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _endapp
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����
	
_endapp:				; void endapp(void);�˳�Ӧ�ó���
	mov	edx,4			;����40h�жϵ��ĺŹ���
	int	0x40
