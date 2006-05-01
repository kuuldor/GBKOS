	.text
	.even

	.globl _IntlDispatch
_IntlDispatch:
	move.l %d2,-(%sp)
	bsr.w getFuncTable
	move.l (%sp)+,%d2
	moveq.l #0,%d0
	move.w %d2,%d0
	lsl.l #2,%d0
	move.l (%a0,%d0.l),%a0
	jmp (%a0)
