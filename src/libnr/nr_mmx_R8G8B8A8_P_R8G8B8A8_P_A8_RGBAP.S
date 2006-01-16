	.file	"nr-compose.c"

# Ensure Inkscape is execshield protected
	.section .note.GNU-stack
	.previous
	
	.text
	.align 2
.globl nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP
	.type	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP,@function

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
 * mm1 MMMM
 * mm2 FgM
 * mm3
 * mm4
 * mm5 255
 * mm6 128
 * mm7 0
 *
*/

nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP:
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

/* Load %mm5 with [255 255 255 255] */
	movl	$0xffffffff, %eax
	movd	%eax, %mm5
	punpcklbw %mm7, %mm5

/* FgC -> %mm0 */
	movl	32(%ebp), %eax
	movd	(%eax), %mm0
	punpcklbw %mm7, %mm0

/* Check full opacity */
	cmpb	$0xff, %al
	jz	.opaque

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

/* [255 - FgA] -> mm1 */
	movq	%mm2, %mm1
	punpckhwd %mm1, %mm1
	punpckhdq %mm1, %mm1
	pxor	%mm5, %mm1

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

.opaque:
/* for (y = ...) */
	movl    16(%ebp), %ecx
.o_fory:

/* d = px */
/* s = spx */
	movl	8(%ebp), %edi
	movl	24(%ebp), %esi

/* for (x = ...) */
	movl	12(%ebp), %ebx
.o_forx:

/* [m m m m] -> %mm1 */
	movzbl	(%esi), %eax
	testb	$0xff, %al
	jz	.o_clip
	cmpb	$0xff, %al
	jz	.o_full
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

/* [255 - FgA] -> mm1 */
	movq	%mm2, %mm1
	punpckhwd %mm1, %mm1
	punpckhdq %mm1, %mm1
	pxor	%mm5, %mm1

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

	jmp	.o_store

.o_full:
	movq	%mm0, %mm3

.o_store:	
/* Store pixel */
	packuswb %mm3, %mm3
	movd	%mm3, (%edi)

.o_clip:
	addl	$4, %edi
	incl	%esi

	decl	%ebx
	jnz	.o_forx

	movl	20(%ebp), %eax
	addl	%eax, 8(%ebp)
	movl	28(%ebp), %eax
	addl	%eax, 24(%ebp)

	decl	%ecx
	jnz	.o_fory
	jmp	.exit

.Lfe1:
	.size	nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP,.Lfe1-nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP
	.ident	"GCC: (GNU) 3.2"
