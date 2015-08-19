; Hand written assembly for Windows x64 

	.CODE

Mutex_lock PROC
	; RCX has the argument
	begin:
	mov rax, 0
	mov rbx, 1
	lock cmpxchg [rcx],rbx
	or rax,rax
	jnz begin
	ret
Mutex_lock ENDP
end