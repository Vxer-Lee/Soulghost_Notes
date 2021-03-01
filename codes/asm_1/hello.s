	.section	__TEXT,__text,regular,pure_instructions
	.build_version ios, 14, 3	sdk_version 14, 3
	.globl	_test                   ; -- Begin function test
	.p2align	2
_test:                                  ; @test
	.cfi_startproc
; %bb.0:
 
;    int test(int a,int b)
;	{
;		int res = a + b;
;		return res;
;	}
	sub	sp, sp, #16             ; =16 开辟16字节空间 sp = sp - 16 ↓
	.cfi_def_cfa_offset 16
	str	w0, [sp, #12] ;(sp+12)=&w0 int a 参数a
	str	w1, [sp, #8]  ;(sp+8)=&w1  int b 参数b
	ldr	w8, [sp, #12] ;w8的值 = (sp+12)
	ldr	w9, [sp, #8]  ;w9的值 = (sp+8)
	add	w8, w8, w9   ; w8 = w8 + w9
	str	w8, [sp, #4]  ;(sp+4)=&w8 = w8 + w9 的结果 int res
	ldr	w0, [sp, #4]  ;取出sp+4的值给w0 w0一般也当做返回值.
	add	sp, sp, #16             ; =16 回收16字节空间 sp = sp + 16 ↑
	ret;返回
	.cfi_endproc
                                        ; -- End function
	.globl	_main                   ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32             ; =32
	stp	x29, x30, [sp, #16]     ; 16-byte Folded Spill
	add	x29, sp, #16            ; =16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	w8, #0
	stur	wzr, [x29, #-4]
	mov	w0, #1
	mov	w1, #2
	str	w8, [sp, #4]            ; 4-byte Folded Spill
	bl	_test
	str	w0, [sp, #8]
	ldr	w0, [sp, #4]            ; 4-byte Folded Reload
	ldp	x29, x30, [sp, #16]     ; 16-byte Folded Reload
	add	sp, sp, #32             ; =32
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
