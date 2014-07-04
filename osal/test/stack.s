	.file	"stack.c"
	.text
	.type	stub_MyCar_drive, @function
stub_MyCar_drive:
.LFB0:
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
	movabsq	$-2401053088876216593, %r12
	.cfi_offset 3, -32
	.cfi_offset 12, -24
	movq	$stub_MyCar_drive, -32(%rbp)
	movq	-32(%rbp), %rax
#APP
# 13 "stack.c" 1
	mov %rax, %rbx
mov %rbx, -0x8(%rbp)

# 0 "" 2
#NO_APP
	movq	$MyCar_drive, -24(%rbp)
	movl	-36(%rbp), %eax
	movq	-24(%rbp), %rdx
	movl	%eax, %esi
	movq	%r12, %rdi
	movl	$0, %eax
	call	*%rdx
	addq	$32, %rsp
	popq	%rbx
	popq	%r12
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	stub_MyCar_drive, .-stub_MyCar_drive
	.type	MyCar_drive, @function
MyCar_drive:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movsd	(%rax), %xmm0
	xorpd	%xmm1, %xmm1
	ucomisd	%xmm1, %xmm0
	seta	%al
	testb	%al, %al
	je	.L2
	movq	-8(%rbp), %rax
	movsd	(%rax), %xmm1
	movl	-28(%rbp), %eax
	testq	%rax, %rax
	js	.L4
	cvtsi2sdq	%rax, %xmm0
	jmp	.L5
.L4:
	movq	%rax, %rdx
	shrq	%rdx
	andl	$1, %eax
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
.L5:
	movsd	.LC1(%rip), %xmm2
	mulsd	%xmm2, %xmm0
	movsd	.LC2(%rip), %xmm2
	addsd	%xmm2, %xmm0
	movapd	%xmm1, %xmm2
	subsd	%xmm0, %xmm2
	movapd	%xmm2, %xmm0
	movq	-8(%rbp), %rax
	movsd	%xmm0, (%rax)
.L2:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	MyCar_drive, .-MyCar_drive
	.globl	MyCar_new
	.type	MyCar_new, @function
MyCar_new:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movsd	%xmm0, -24(%rbp)
	movl	$16, %edi
	call	malloc
	movq	%rax, -16(%rbp)
	movl	$0, -4(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L7
.L8:
	movl	-4(%rbp), %eax
	addq	-16(%rbp), %rax
	movb	$0, (%rax)
	addl	$1, -4(%rbp)
.L7:
	cmpl	$15, -4(%rbp)
	jbe	.L8
	movq	-16(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-16(%rbp), %rax
	movq	%rax, %rdx
	movl	$512, %esi
	movl	$stub_MyCar_drive, %edi
	call	coo_patch
	movq	%rax, %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, 8(%rax)
	movq	-16(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	MyCar_new, .-MyCar_new
	.globl	main
	.type	main, @function
main:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -36(%rbp)
	movq	%rsi, -48(%rbp)
	movl	$0, %eax
	call	LinuxMemoryNew
	cltq
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	LinuxSpecialMemoryNew
	cltq
	movq	%rax, -16(%rbp)
	movq	$0, -8(%rbp)
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	coo_init
	movsd	.LC3(%rip), %xmm0
	call	MyCar_new
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movl	$10, %edi
	call	*%rax
	movl	$0, %eax
	call	coo_end
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	main, .-main
	.section	.rodata
	.align 8
.LC1:
	.long	2576980378
	.long	1070176665
	.align 8
.LC2:
	.long	0
	.long	1071644672
	.align 8
.LC3:
	.long	0
	.long	1076101120
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
