[bits 32]
section .text
extern put_char
global put_str
put_str:
	push ebx
	push ecx
	xor ecx,ecx
	mov ebx,[esp+12]
	.goon:
		mov cl,[ebx]
		cmp cl,0
		jz .str_over
		push ecx
		call put_char
		add esp,4
		inc ebx
		jmp .goon
	.str_over:
		pop ecx
		pop ebx
		ret
