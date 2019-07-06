bits 64
section .text

global write_integer
global exit

write_integer:
	push rbp
	mov rbp, rsp

	mov rcx, 10
	
	dec rsp
	mov [rsp], cl

.loop:
	xor rdx, rdx
	div rcx
	add edx, 48

	dec rsp
	mov [rsp], dl

	cmp rax, 0
	jne .loop

	mov rdx, rbp
	sub rdx, rsp ; count
	mov rsi, rsp ; buffer
	mov rdi, 1 ; stdout
	mov rax, 1 ; write
	syscall

	mov rsp, rbp
	pop rbp
	ret

exit:
	xor rdi, rdi
	mov rax, 60
	syscall

