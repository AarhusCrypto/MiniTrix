; Handwritten Assembly for Win x64 CooV4 support
	.CODE

COO_internal_getThis PROC
	mov rax, r10
	ret
COO_internal_getThis ENDP

generic_stub PROC
	call unique_label_JnMhnwRGLG
	unique_label_JnMhnwRGLG:
	pop QWORD PTR rax
	shr rax, 5
	shl rax, 5
	mov r10,QWORD PTR 8[rax]
	mov rax,QWORD PTR [rax]
	jmp rax
generic_stub ENDP

end