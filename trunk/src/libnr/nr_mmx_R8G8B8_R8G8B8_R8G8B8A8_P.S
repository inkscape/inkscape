	.file	"nr-compose.c"

# Ensure Inkscape is execshield protected
	.section .note.GNU-stack
	.previous
	
	.text
	.align 2
.globl nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P
	.type	nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P,@function

/*
 * This code is in public domain
 *
 * alpha 32(%ebp)
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
 * mm0 A
 * mm1 FgA
 * mm2 FgPre
 * mm3
 * mm4
 * mm5 255
 * mm6 128
 * mm7 0
 *
*/

nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P:
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

/* Load %mm0 with [a a a a] */
/* Check full opacity */
	movzbl	32(%ebp), %eax
	cmpb	$0xff, %al
	jz	.opaque
	movd	%eax, %mm0
	punpcklwd %mm0, %mm0
	punpckldq %mm0, %mm0

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

/* Fg -> %mm1 */
/* fixme: Do we have to bother about alignment here? (Lauris) */
	movl	(%esi), %eax
	testl	$0xff000000, %eax
	jz	.clip
	movd	%eax, %mm1
	punpcklbw %mm7, %mm1

/* [Fg * a] -> mm1 */
	pmullw	%mm0, %mm1
	paddw	%mm6, %mm1
	movq	%mm1, %mm2
	psrlw	$8, %mm2
	paddw	%mm2, %mm1
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
	movd	%mm3, %eax
	movb	%al, 0(%edi)
	shrl	$8, %eax
	movb	%al, 1(%edi)
	shrl	$8, %eax
	movb	%al, 2(%edi)

.clip:
	addl	$3, %edi
	addl	$4, %esi

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

/* Fg -> %mm1 */
/* fixme: Do we have to bother about alignment here? (Lauris) */
	movl	(%esi), %eax
	testl	$0xff000000, %eax
	jz	.o_clip
	cmpl	$0xff000000, %eax
	jnb	.o_store
	movd	%eax, %mm1
	punpcklbw %mm7, %mm1

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
	movd	%mm3, %eax
.o_store:
	movb	%al, 0(%edi)
	shrl	$8, %eax
	movb	%al, 1(%edi)
	shrl	$8, %eax
	movb	%al, 2(%edi)

.o_clip:
	addl	$3, %edi
	addl	$4, %esi

	decl	%ebx
	jnz	.o_forx

	movl	20(%ebp), %eax
	addl	%eax, 8(%ebp)
	movl	28(%ebp), %eax
	addl	%eax, 24(%ebp)

	decl	%ecx
	jnz	.o_fory

	jmp .exit

.Lfe1:
	.size	nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P,.Lfe1-nr_mmx_R8G8B8_R8G8B8_R8G8B8A8_P
	.ident	"GCC: (GNU) 3.2"
