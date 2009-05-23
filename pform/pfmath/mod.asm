	BITS	32
	GLOBAL	_Imod
	GLOBAL	_Imod2
	GLOBAL	_Imodtest

	SECTION	.text
		
;inline static I_TYPE Imod(I_TYPE * a, const I_TYPE n, const int len)
_Imod:
Imod:
	push	ebp
	mov	ebp,esp

	push	edx	
	push	ebx
	xor	edx,edx
	push	ecx
	mov	ebx,[ebp+16]	; len

	mov	ecx,[ebp+8]		; a

.loop
	mov	eax,[ecx+ebx*4-4]
	div	dword [ebp+12]
	dec	ebx
	jnz	.loop

	mov	eax,edx
	pop	ecx
	pop	ebx	
	pop	edx
		
	pop	ebp
	ret

;inline static I_TYPE Imod2(
;I_TYPE * a,			[8]
;const I_TYPE m1		[12]
;const I_TYPE m2		[16]
;const int len)		[20]
;I_TYPE *x1				[24]
;I_TYPE *x2				[28]

_Imod2:
Imod2:
	push	ebp
	mov	ebp,esp

	push	eax
	push	ebx
	push	ecx
	push	edx
	mov	ebx,[ebp+20]	; len
	mov	ecx,[ebp+8]		; a
	
	fild	dword [ebp+12]		;m1
	fld1					;1, m1
	fdiv	st0,st1		;1/m1, m1
	fild	dword [ebp+16]		;m2, 1/m1, m1
	fld1					;1, m2, 1/m1, m1
	fdiv	st0,st1		;1/m2, m2, 1/m1, m1
	fldz					;
	fldz					;x2, x1, 1/m2, m2, 1/m1, m1
.loop
; since the result on the previous iteration could be negative

; doesnt seem to make any difference if this removed
; however I am leaving it in just in case

	fxch	st1
	fadd	st0,st5
	fxch	st1
	fadd	st0,st3

; first multiply by 2^32

	fild	dword [D65536]
	mov	edx,[ecx+ebx*4-4]
	fmul	st0,st0			; 2^32 x2 x1 (recips)

	and	edx,edx
; this bit does make a difference if removed

	jns	.ispositive
	fld1
	fadd	st3,st0
	faddp	st2,st0

.ispositive
	fmul	st2,st0
	fmulp	st1,st0		; x2<<, x1<<
	
	fild	dword [ecx+ebx*4-4]
	fadd	st2,st0
	faddp	st1,st0		; x2 x1 1/m2 m2 1/m1 m1
	
	fld	st0			; x2 x2 x1 1/m2 m2 1/m1 m1
	fmul	st0,st3		; x2/m2 x2 x1 1/m2 m2 1/m1 m1
	fld	st2			; x1 x2/m2 x2 x1 1/m2 m2 1/m1 m1
	fmul	st0,st6		; x1/m1 x2/x2 x2 x1 1/m2 m2 1/m1 m1

; round
	fxch	st1			;x2*x2/m2#3, x1*x1/m1#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fadd	qword	[C6263]	;x2*x2/m2#1, x1*x1/m1#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;x1*x1/m1#2, x2*x2/m2#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fadd	qword [C6263]	;x1*x1/m1#1, x2*x2/m2#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;x2*x2/m2#2, x1*x1/m2#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fsub	qword [C6263]	;[x2*x2/m2]#1, x1*x1/m1#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;x1*x1/m1#2, [x2*x2/m2]#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fsub	qword [C6263]	;[x1*x1/m1]#1, [x2*x2/m2]#2, x2*x2, x1*x1, m2r, m2, m1r, m1

;	fxch	st1
;	frndint
;	fxch	st1
;	frndint
	
	fxch	st1
	
	fmul	st0, st5		;[x2*x2]#1, [x1*x1/m1]#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;[x1*x1/m1]#2, [x2*x2]#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fmul	st0, st7		;[x1*x1]#1, [x2*x2]#3, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;[x2*x2]#3, [x1*x1]#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fsubp	st2, st0		;[x1*x1]#2, x2#1, 2*x1*x1, m2r, m2, m1r, m1
	dec	ebx
	fsubp	st2, st0		;x2#3, x1#1, m2r, m2, m1r, m1
	jnz	.loop
	
	fxch	st5			;m1, x1#2, m2r, m2, m1r, x2#1
	mov	eax,[ebp+28]
	fstp	st0			;x1#3, m2r, m2, m1r, x2#2
	mov	ebx,[ebp+24]
	fxch	st3			;m1r, m2r, m2, x1#3, x2#2
	fstp	st0			;m2r, m2, x1, x2#3
	fcompp

	pop	edx
	fistp	dword [ebx]
	pop	ecx
	fistp	dword [eax]
	pop	ebx
	pop	eax
		
	pop	ebp
	ret

;void Imodtest(DWORD x,DWORD y,DWORD *p1, DWORD *p2)
_Imodtest:
Imodtest
	push	ebp
	mov	ebp,esp

	push	ebx
	push	edx	
	push	ecx
	xor	edx,edx
	
	mov	eax,[ebp+16]
	mov	ebx,[ebp+20]

	fild	dword [ebp+8]		;m1
	fld1					;1, m1
	fdiv	st0,st1		;1/m1, m1
	fild	dword [ebp+12]		;m2, 1/m1, m1
	fld1					;1, m2, 1/m1, m1
	fdiv	st0,st1		;1/m2, m2, 1/m1, m1
	
	fild	dword [eax]
	fild	dword [ebx]

	fld	st0			; x2 x2 x1 1/m2 m2 1/m1 m1
	fmul	st0,st3		; x2/m2 x2 x1 1/m2 m2 1/m1 m1
	fld	st2			; x1 x2/m2 x2 x1 1/m2 m2 1/m1 m1
	fmul	st0,st6		; x1/m1 x2/x2 x2 x1 1/m2 m2 1/m1 m1

; round
	fxch	st1			;x2*x2/m2#3, x1*x1/m1#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fadd	qword	[C6263]	;x2*x2/m2#1, x1*x1/m1#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;x1*x1/m1#2, x2*x2/m2#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fadd	qword [C6263]	;x1*x1/m1#1, x2*x2/m2#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;x2*x2/m2#2, x1*x1/m2#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fsub	qword [C6263]	;[x2*x2/m2]#1, x1*x1/m1#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;x1*x1/m1#2, [x2*x2/m2]#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fsub	qword [C6263]	;[x1*x1/m1]#1, [x2*x2/m2]#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1
	
	fmul	st0, st5		;[x2*x2]#1, [x1*x1/m1]#2, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;[x1*x1/m1]#2, [x2*x2]#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fmul	st0, st7		;[x1*x1]#1, [x2*x2]#3, x2*x2, x1*x1, m2r, m2, m1r, m1
	fxch	st1			;[x2*x2]#3, [x1*x1]#1, x2*x2, x1*x1, m2r, m2, m1r, m1
	fsubp	st2, st0		;[x1*x1]#2, x2#1, 2*x1*x1, m2r, m2, m1r, m1
	fsubp	st2, st0		;x2#3, x1#1, m2r, m2, m1r, m1
	
	fxch	st5			;m1, x1#2, m2r, m2, m1r, x2#1
	fstp	st0			;x1#3, m2r, m2, m1r, x2#2
	fxch	st3			;m1r, m2r, m2, x1#3, x2#2
	fstp	st0			;m2r, m2, x1, x2#3
	fcompp

	pop	edx
	fistp	dword [eax]
	pop	ecx
	fistp	dword [ebx]
	pop	ebx	
	pop	eax
	
	pop	ebp
	ret
	
	SECTION .data
C6263:
	db	0,0,0,0,0,0,0xe8,0x43
D65536:
	dd	65536


