	.file	"nr-compose.c"
	
# Ensure Inkscape is execshield protected
	.section .note.GNU-stack
	.previous
	
	.text
	.align 2
.globl nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP
	.type	nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP,@function

/*
 * This code is in public domain
 *
 * c	 32(%ebp)
 * srs	 28(%ebp)
 * spx	 24(%ebp)
 * rs	 20(%ebp)
 * h	 16(%ebp)
 * w	 12(%ebp)
 * px	 8(%ebp)
 * r	-8(%ebp)
 * g	-12(%ebp)
 * b	-16(%ebp)
 * a	-20(%ebp)
 * s	-24(%ebp) -> %esi
 * d	-28(%ebp) -> %edi
 * x	-32(%ebp) -> %ebx
 * y	-36(%ebp)
 * ca	-40(%ebp)
 *
 * mm0 Fg
 * mm1 FgA
 * mm2 FgPre
 * mm3
 * mm4
 * mm5
 * mm6 128
 * mm7 0
 *
*/

nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$36, %esp
	pushl	%edi
	pushl	%esi

/* Load %mm7 with [0 0 0 0] */
	movl	$0, %eax
	movd	%eax, %mm7

/* Load %mm6 with [128 128 128 128] */
	movl	$0x80808080, %eax
	movd	%eax, %mm6
	punpcklbw %mm7, %mm6

/* FgC -> %mm0 */
	movl	32(%ebp), %eax
	movd	(%eax), %mm0
	punpcklbw %mm7, %mm0

/* for (y = ...) */
	movl    16(%ebp), %ecx
.fory:

/* d = px */
/* s = spx */
	movl	8(%ebp), %edi
	movl	24(%ebp), %esi

/* for (x = ...) */
	movl	12(%ebp), %ebx
.forx:

/* [m m m m] -> %mm1 */
	movzbl	(%esi), %eax
	testb	$0xff, %al
	jz	.clip
	movd	%eax, %mm1
	punpcklwd %mm1, %mm1
	punpckldq %mm1, %mm1

/* Fg -> mm2 */
	movq	%mm0, %mm2
	pmullw	%mm1, %mm2
	paddw	%mm6, %mm2
	movq	%mm2, %mm3
	psrlw	$8, %mm3
	paddw	%mm3, %mm2
	psrlw	$8, %mm2

/* Store pixel */
	packuswb %mm2, %mm2
	movd	%mm2, (%edi)

.clip:
	addl	$4, %edi
	incl	%esi

	decl	%ebx
	jnz	.forx

	movl	20(%ebp), %eax
	addl	%eax, 8(%ebp)
	movl	28(%ebp), %eax
	addl	%eax, 24(%ebp)

	decl	%ecx
	jnz	.fory

.exit:
	emms
	popl	%esi
	popl	%edi
	addl	$36, %esp
	popl	%ebx
	popl	%ebp
	ret

.Lfe1:
	.size	nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP,.Lfe1-nr_mmx_R8G8B8A8_P_EMPTY_A8_RGBAP
	.ident	"GCC: (GNU) 3.2"
