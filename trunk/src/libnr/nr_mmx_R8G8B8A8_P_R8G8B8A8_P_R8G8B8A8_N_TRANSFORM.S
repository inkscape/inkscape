	.file	"nr-compose-transform.c"

# Ensure Inkscape is execshield protected
	.section .note.GNU-stack
	.previous
	
	.text
	.align 2
.globl nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_0
	.type	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_0,@function

/*
 * This code is in public domain
 *
 */

nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_0:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$48, %esp
	pushl	%edi
	pushl	%esi

/* Load %mm7 with [0 0 0 0] */
	movl	$0, %eax
	movd	%eax, %mm7

/* Load %mm6 with [128 128 128 128] */
	movl	$0x80808080, %eax
	movd	%eax, %mm6
	punpcklbw %mm7, %mm6

/* Load %mm5 with [255 255 255 255] */
	movl	$0xffffffff, %eax
	movd	%eax, %mm5
	punpcklbw %mm7, %mm5

/* Load %mm0 with [a a a a] */
	movzbl	44(%ebp), %eax
	movd	%eax, %mm0
	punpcklwd %mm0, %mm0
	punpckldq %mm0, %mm0

	movl	8(%ebp), %eax
	movl	%eax, -8(%ebp)
	movl	40(%ebp), %eax
	addl	$16, %eax
	movl	(%eax), %eax
	movl	%eax, -12(%ebp)
	movl	40(%ebp), %eax
	addl	$20, %eax
	movl	(%eax), %eax
	movl	%eax, -16(%ebp)
	movl	$0, -24(%ebp)
.L29:
	movl	-24(%ebp), %eax
	cmpl	16(%ebp), %eax
	jl	.L32
	jmp	.L28
.L32:
	movl	-8(%ebp), %edi

	movl	-12(%ebp), %eax
	movl	%eax, %esi
	movl	-16(%ebp), %eax
	movl	%eax, -36(%ebp)

	movl	12(%ebp), %ebx
.for_x_0:

	movl	%esi, %ecx
	cmpl	$0, %ecx
	js	.clip_0
	sarl	$12, %ecx
	cmpl	28(%ebp), %ecx
	jge	.clip_0
	shll	$2, %ecx

	movl	-36(%ebp), %eax
	cmpl	$0, %eax
	js	.clip_0
	sarl	$12, %eax
	cmpl	32(%ebp), %eax
	jge	.clip_0
	imull	36(%ebp), %eax

	addl	%ecx, %eax
	addl	24(%ebp), %eax

/* Fg -> %mm1 */
	movl	(%eax), %eax
	testl	$0xff000000, %eax
	jz	.clip_0
	movd	%eax, %mm1
	punpcklbw %mm7, %mm1

/* [a a a 255] -> %mm3 */
	shrl	$24, %eax
	movl	$0x10101, %edx
	mull	%edx
	orl	$0xff000000, %eax
	movd	%eax, %mm3
	punpcklbw %mm7, %mm3

/* [Fg * a] -> mm1 */
	pmullw	%mm3, %mm1
	paddw	%mm6, %mm1
	movq	%mm1, %mm4
	psrlw	$8, %mm4
	paddw	%mm4, %mm1
	psrlw	$8, %mm1

/* Multiply by alpha */
	pmullw	%mm0, %mm1
	paddw	%mm6, %mm1
	movq	%mm1, %mm4
	psrlw	$8, %mm4
	paddw	%mm4, %mm1
	psrlw	$8, %mm1

/* [255 - FgA] -> mm2 */
	movq	%mm1, %mm2
	punpckhwd %mm2, %mm2
	punpckhdq %mm2, %mm2
	pxor	%mm5, %mm2

/* Bg -> mm3 */
	movd	(%edi), %mm3
	punpcklbw %mm7, %mm3

/* Fg + ((255 - FgA) * Bg) / 255 */

	pmullw	%mm2, %mm3
	paddw	%mm6, %mm3
	movq	%mm3, %mm4
	psrlw	$8, %mm4
	paddw	%mm4, %mm3
	psrlw	$8, %mm3
	paddw	%mm1, %mm3

/* Store pixel */
	packuswb %mm3, %mm3
	movd	%mm3, (%edi)

.clip_0:
.L37:
	movl	40(%ebp), %ecx
	movl	(%ecx), %edx
	addl	%edx, %esi
	movl	4(%ecx), %edx
	addl	%edx, -36(%ebp)

	addl	$4, %edi

	decl	%ebx
	jnz	.for_x_0

.L34:
	movl	8(%ecx), %edx
	addl	%edx, -12(%ebp)
	movl	12(%ecx), %edx
	addl	%edx, -16(%ebp)

	movl	20(%ebp), %edx
	leal	-8(%ebp), %eax
	addl	%edx, (%eax)
	leal	-24(%ebp), %eax
	incl	(%eax)
	jmp	.L29
.L28:
	emms
	popl	%esi
	popl	%edi
	addl	$48, %esp
	popl	%ebx
	popl	%ebp
	ret
.Lfe2:
	.size	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_0,.Lfe2-nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_0

/*
 *
 *	dbits	52(%ebp)
 *	alpha	48(%ebp)
 *	FF_S	44(%ebp)
 *
 *	d	-32(%ebp) -> %edi
 *	i	-60(%ebp) -> %esi
 *	sx	-64(%ebp) -> %ebx
 *	sy	-68(%ebp)
 *	s	-72(%ebp)
 *
 *	%mm0	a a a a
 *	%mm1	FgA
 *	%mm2	SumFgA
 *	%mm3	a a a 255
 *	%mm4
*/

	.align 2
.globl nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_n
	.type	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_n,@function
nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_n:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$72, %esp
	pushl	%edi
	pushl	%esi

/* Load %mm7 with [0 0 0 0] */
	movl	$0, %eax
	movd	%eax, %mm7

/* Load %mm6 with [128 128 128 128] */
	movl	$0x80808080, %eax
	movd	%eax, %mm6
	punpcklbw %mm7, %mm6

/* Load %mm5 with [255 255 255 255] */
	movl	$0xffffffff, %eax
	movd	%eax, %mm5
	punpcklbw %mm7, %mm5

/* Load %mm0 with [a a a a] */
	movzbl	48(%ebp), %eax
	movd	%eax, %mm0
	punpcklwd %mm0, %mm0
	punpckldq %mm0, %mm0

	movl	$1, %eax
	movzbl	52(%ebp), %ecx
	sall	%cl, %eax
	movl	%eax, -8(%ebp)
	movl	8(%ebp), %eax
	movl	%eax, -12(%ebp)
	movl	40(%ebp), %eax
	addl	$16, %eax
	movl	(%eax), %eax
	movl	%eax, -16(%ebp)
	movl	40(%ebp), %eax
	addl	$20, %eax
	movl	(%eax), %eax
	movl	%eax, -20(%ebp)
	movl	$0, -28(%ebp)
.L44:
	movl	-28(%ebp), %eax
	cmpl	16(%ebp), %eax
	jl	.L47
	jmp	.exit_n
.L47:
	movl	-12(%ebp), %eax
	movl	%eax, -32(%ebp)
	movl	-16(%ebp), %eax
	movl	%eax, -36(%ebp)
	movl	-20(%ebp), %eax
	movl	%eax, -40(%ebp)
	movl	$0, -24(%ebp)
.L48:
	movl	-24(%ebp), %eax
	cmpl	12(%ebp), %eax
	jl	.L51
	jmp	.L49
.L51:

/* Zero accumulator */
	movq	%mm7, %mm2

/* Set i to dptr (size - 1) */
	movl	-8(%ebp), %esi
	sub	$1, %esi
	shll	$3, %esi

	movl	44(%ebp), %edi
	movl	-36(%ebp), %ecx

.for_i_n:
	movl	(%edi,%esi), %ebx
	addl	%ecx, %ebx
/* Test negative before shift */
	cmpl	$0, %ebx
	js	.next_i_n
	sarl	$12, %ebx
	cmpl	28(%ebp), %ebx
	jge	.next_i_n
/* We multiply sx by 4 here */
	shll	$2, %ebx

	movl	4(%edi,%esi), %eax
	addl	-40(%ebp), %eax
/* Test negative before shift */
	cmpl	$0, %eax
	js	.next_i_n
	sarl	$12, %eax
	cmpl	32(%ebp), %eax
	jge	.next_i_n
/* We multiply sy by srs here */
	imull	36(%ebp), %eax

	addl	%ebx, %eax
	addl	24(%ebp), %eax

/* Fg -> %mm1 */
	movl	(%eax), %eax
	testl	$0xff000000, %eax
	jz	.next_i_n
	movd	%eax, %mm1
	punpcklbw %mm7, %mm1

/* [a a a 255] -> %mm3 */
	shrl	$24, %eax
	movl	$0x10101, %edx
	mull	%edx
	orl	$0xff000000, %eax
	movd	%eax, %mm3
	punpcklbw %mm7, %mm3

/* [Fg * a] -> mm1 */
	pmullw	%mm3, %mm1
	paddw	%mm6, %mm1
	movq	%mm1, %mm4
	psrlw	$8, %mm4
	paddw	%mm4, %mm1
	psrlw	$8, %mm1

/* Add to accumulator */
	paddw	%mm1, %mm2

.next_i_n:
	subl	$8, %esi
	jnb	.for_i_n

/* Divide components by sample size */
	movd	52(%ebp), %mm3
	psrlw	%mm3, %mm2

/* Multiply by alpha */
	pmullw	%mm0, %mm2
	paddw	%mm6, %mm2
	movq	%mm2, %mm4
	psrlw	$8, %mm4
	paddw	%mm4, %mm2
	psrlw	$8, %mm2

/* [255 - FgA] -> mm1 */
	movq	%mm2, %mm1
	punpckhwd %mm1, %mm1
	punpckhdq %mm1, %mm1
	pxor	%mm5, %mm1

	movl	-32(%ebp), %edi
/* Bg -> mm3 */
	movd	(%edi), %mm3
	punpcklbw %mm7, %mm3

/* Fg + ((255 - FgA) * Bg) / 255 */

	pmullw	%mm1, %mm3
	paddw	%mm6, %mm3
	movq	%mm3, %mm4
	psrlw	$8, %mm4
	paddw	%mm4, %mm3
	psrlw	$8, %mm3
	paddw	%mm2, %mm3

/* Store pixel */
	packuswb %mm3, %mm3
	movd	%mm3, (%edi)

.L58:
	movl	40(%ebp), %eax
	movl	(%eax), %edx
	leal	-36(%ebp), %eax
	addl	%edx, (%eax)
	movl	40(%ebp), %eax
	addl	$4, %eax
	movl	(%eax), %edx
	leal	-40(%ebp), %eax
	addl	%edx, (%eax)
	leal	-32(%ebp), %eax
	addl	$4, (%eax)
	leal	-24(%ebp), %eax
	incl	(%eax)
	jmp	.L48
.L49:
	movl	40(%ebp), %eax
	addl	$8, %eax
	movl	(%eax), %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
	movl	40(%ebp), %eax
	addl	$12, %eax
	movl	(%eax), %edx
	leal	-20(%ebp), %eax
	addl	%edx, (%eax)
	movl	20(%ebp), %edx
	leal	-12(%ebp), %eax
	addl	%edx, (%eax)
	leal	-28(%ebp), %eax
	incl	(%eax)
	jmp	.L44

.exit_n:
	emms
	popl	%esi
	popl	%edi
	addl	$72, %esp
	popl	%ebx
	popl	%ebp
	ret
.Lfe3:
	.size	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_n,.Lfe3-nr_mmx_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM_n
	.ident	"GCC: (GNU) 3.2"
