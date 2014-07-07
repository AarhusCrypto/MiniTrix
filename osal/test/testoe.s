	.file	"testoe.c"
	.section	.rodata
.LC0:
	.string	"ID from thread %llu\n"
	.text
	.globl	ts
	.type	ts, @function
ts:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, -8(%rbp)
	movl	$1, %edi
	call	sleep
	call	pthread_self
	movq	%rax, %rdx
	movl	$.LC0, %eax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	ts, .-ts
	.type	stub_SObj_magic, @function
stub_SObj_magic:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r12
	pushq	%rbx
	subq	$32, %rsp
	movl	%edi, -36(%rbp)
	movabsq	$-2401053088876216593, %rbx
	.cfi_offset 3, -32
	.cfi_offset 12, -24
	movl	$SObj_magic, %r12d
	movl	-36(%rbp), %eax
	movl	%eax, %esi
	movq	%rbx, %rdi
	movl	$0, %eax
	call	*%r12
	movl	%eax, -20(%rbp)
	movl	-20(%rbp), %eax
	addq	$32, %rsp
	popq	%rbx
	popq	%r12
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	stub_SObj_magic, .-stub_SObj_magic
	.type	SObj_magic, @function
SObj_magic:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	-12(%rbp), %eax
	addl	$66, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	SObj_magic, .-SObj_magic
	.globl	SObj_new
	.type	SObj_new, @function
SObj_new:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$8, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdx
	movl	$512, %esi
	movl	$stub_SObj_magic, %edi
	call	coo_patch
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	SObj_new, .-SObj_new
	.section	.rodata
.LC1:
	.string	"%s\tfailed"
.LC2:
	.string	"oe->yieldthread"
.LC3:
	.string	"%s\tok"
.LC4:
	.string	"oe->accept"
.LC5:
	.string	"oe->getmem"
.LC6:
	.string	"oe->putmem"
.LC7:
	.string	"oe->read"
.LC8:
	.string	"oe->write"
.LC9:
	.string	"oe->open"
.LC10:
	.string	"oe->close"
.LC11:
	.string	"oe->newthread"
.LC12:
	.string	"oe->jointhread"
.LC13:
	.string	"oe->newmutex"
.LC14:
	.string	"oe->destroymutex"
.LC15:
	.string	"oe->lock"
.LC16:
	.string	"oe->unlock"
.LC17:
	.string	"oe->newsemaphore"
.LC18:
	.string	"oe->destroysemaphore"
.LC19:
	.string	"oe->down"
.LC20:
	.string	"oe->up"
.LC21:
	.string	"oe->syslog"
.LC22:
	.string	"oe->p"
.LC23:
	.string	"%llu \n"
.LC24:
	.string	"leaving ... "
	.text
	.globl	main
	.type	main, @function
main:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$0, %eax
	call	OperatingEnvironment_LinuxNew
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movq	72(%rax), %rax
	testq	%rax, %rax
	jne	.L6
	movl	$.LC1, %eax
	movl	$.LC2, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L7
.L6:
	movl	$.LC3, %eax
	movl	$.LC2, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L7:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	56(%rax), %rax
	testq	%rax, %rax
	jne	.L8
	movl	$.LC1, %eax
	movl	$.LC4, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L9
.L8:
	movl	$.LC3, %eax
	movl	$.LC4, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L9:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	8(%rax), %rax
	testq	%rax, %rax
	jne	.L10
	movl	$.LC1, %eax
	movl	$.LC5, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L11
.L10:
	movl	$.LC3, %eax
	movl	$.LC5, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L11:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	16(%rax), %rax
	testq	%rax, %rax
	jne	.L12
	movl	$.LC1, %eax
	movl	$.LC6, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L13
.L12:
	movl	$.LC3, %eax
	movl	$.LC6, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L13:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	24(%rax), %rax
	testq	%rax, %rax
	jne	.L14
	movl	$.LC1, %eax
	movl	$.LC7, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L15
.L14:
	movl	$.LC3, %eax
	movl	$.LC7, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L15:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	32(%rax), %rax
	testq	%rax, %rax
	jne	.L16
	movl	$.LC1, %eax
	movl	$.LC8, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L17
.L16:
	movl	$.LC3, %eax
	movl	$.LC8, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L17:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	40(%rax), %rax
	testq	%rax, %rax
	jne	.L18
	movl	$.LC1, %eax
	movl	$.LC9, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L19
.L18:
	movl	$.LC3, %eax
	movl	$.LC9, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L19:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	48(%rax), %rax
	testq	%rax, %rax
	jne	.L20
	movl	$.LC1, %eax
	movl	$.LC10, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L21
.L20:
	movl	$.LC3, %eax
	movl	$.LC10, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L21:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	64(%rax), %rax
	testq	%rax, %rax
	jne	.L22
	movl	$.LC1, %eax
	movl	$.LC11, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L23
.L22:
	movl	$.LC3, %eax
	movl	$.LC11, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L23:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	80(%rax), %rax
	testq	%rax, %rax
	jne	.L24
	movl	$.LC1, %eax
	movl	$.LC12, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L25
.L24:
	movl	$.LC3, %eax
	movl	$.LC12, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L25:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	104(%rax), %rax
	testq	%rax, %rax
	jne	.L26
	movl	$.LC1, %eax
	movl	$.LC13, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L27
.L26:
	movl	$.LC3, %eax
	movl	$.LC13, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L27:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	128(%rax), %rax
	testq	%rax, %rax
	jne	.L28
	movl	$.LC1, %eax
	movl	$.LC14, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L29
.L28:
	movl	$.LC3, %eax
	movl	$.LC14, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L29:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	112(%rax), %rax
	testq	%rax, %rax
	jne	.L30
	movl	$.LC1, %eax
	movl	$.LC15, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L31
.L30:
	movl	$.LC3, %eax
	movl	$.LC15, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L31:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	120(%rax), %rax
	testq	%rax, %rax
	jne	.L32
	movl	$.LC1, %eax
	movl	$.LC16, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L33
.L32:
	movl	$.LC3, %eax
	movl	$.LC16, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L33:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	136(%rax), %rax
	testq	%rax, %rax
	jne	.L34
	movl	$.LC1, %eax
	movl	$.LC17, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L35
.L34:
	movl	$.LC3, %eax
	movl	$.LC17, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L35:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	152(%rax), %rax
	testq	%rax, %rax
	jne	.L36
	movl	$.LC1, %eax
	movl	$.LC18, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L37
.L36:
	movl	$.LC3, %eax
	movl	$.LC18, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L37:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	144(%rax), %rax
	testq	%rax, %rax
	jne	.L38
	movl	$.LC1, %eax
	movl	$.LC19, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L39
.L38:
	movl	$.LC3, %eax
	movl	$.LC19, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L39:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	160(%rax), %rax
	testq	%rax, %rax
	jne	.L40
	movl	$.LC1, %eax
	movl	$.LC20, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L41
.L40:
	movl	$.LC3, %eax
	movl	$.LC20, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L41:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	168(%rax), %rax
	testq	%rax, %rax
	jne	.L42
	movl	$.LC1, %eax
	movl	$.LC21, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L43
.L42:
	movl	$.LC3, %eax
	movl	$.LC21, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L43:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	176(%rax), %rax
	testq	%rax, %rax
	jne	.L44
	movl	$.LC1, %eax
	movl	$.LC22, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	jmp	.L45
.L44:
	movl	$.LC3, %eax
	movl	$.LC22, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L45:
	movl	$10, %edi
	call	putchar
	movq	-16(%rbp), %rax
	movq	8(%rax), %rax
	movl	$8, %edi
	call	*%rax
	movq	%rax, -8(%rbp)
	movq	-16(%rbp), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rcx
	movl	$ts, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_create
	movq	-8(%rbp), %rax
	movq	(%rax), %rdx
	movl	$.LC23, %eax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	$2, %edi
	call	sleep
	movl	$.LC24, %eax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
