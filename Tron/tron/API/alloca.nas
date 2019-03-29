[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "alloca.nas"]

		GLOBAL	__alloca

[SECTION .text]

__alloca:				
	ADD	EAX,-4			;由于需要代替ret所以在jmp后还要将esp+4
	SUB	ESP,EAX
	JMP	DWORD [ESP+EAX]		; 代替ret
