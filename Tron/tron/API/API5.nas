;��Ӧ�ó�����õ�����

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ�������ʽ��	
[INSTRSET "i486p"]					
[BITS 32]				;����32λģʽ�õĻ�������
			
					;����Ŀ���ļ�����Ϣ
[FILE "appfunc.nas"]			;Դ�ļ���

	GLOBAL _getfifo
	
[SECTION .text]				;��Ŀ���ļ����ȼ�����Щ��д����

_getfifo:				;int getfifo();���ܻ���������
	mov	edx,5
	int	0x40
	ret