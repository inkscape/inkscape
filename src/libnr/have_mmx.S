	.file "have_mmx.S"

# Ensure Inkscape is execshield protected
	.section .note.GNU-stack
	.previous
	
	.version "01.01"
gcc2_compiled.:
.text
	.align 16
.globl nr_have_mmx
	.type	 nr_have_mmx,@function

nr_have_mmx:
	push	%ebx

# Check if bit 21 in flags word is writeable

	pushfl	
	popl	%eax
	movl	%eax,%ebx
	xorl	$0x00200000, %eax
	pushl   %eax
	popfl
	pushfl
	popl	%eax

	cmpl	%eax, %ebx

	je .notfound

# OK, we have CPUID

	movl	$1, %eax
	cpuid
	
	test	$0x00800000, %edx
	jz	.notfound

	movl	$1, %eax
	jmp	.out

.notfound:
	movl  	$0, %eax
.out:	
	popl	%ebx
	ret
