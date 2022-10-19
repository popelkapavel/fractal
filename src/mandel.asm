; vypocet fraktalu, funkce maji parametry long real
; a globalni struktura sfw definuje maximalni pocet iteraci a bod pro julie

[BITS 32]
[SECTION .text]

[EXTERN _sfw]

x_real equ 8     ; index na zasobnik pro  x a y
y_real equ 20

fw_px   equ 0    ; indexy do struktury fractal window
fw_py   equ 12 
fw_iter equ 24   

[GLOBAL _ff_julia]
[GLOBAL _ff_juliab]
[GLOBAL _ff_mandelbrot]

_ff_juliab:
        push esi
        mov esi,dword [_sfw]
        fild dword [const4]
        fld tword [esp+x_real]
        fld tword [esp+y_real]
        fld tword [esi+fw_px]
        fld tword [esi+fw_py]
        jmp short mandel_start

_ff_julia:
        push esi
        mov esi,dword [_sfw]
        fild dword [const4]
        fld tword [esi+fw_px]
        fld tword [esi+fw_py]
        fld tword [esp+x_real]
        fld tword [esp+y_real]
        jmp short mandel_start

_ff_mandelbrot:
        push esi
        mov esi,dword [_sfw]
        fild dword [const4]    ; bound
        fld tword [esp+x_real] ; cx
        fld tword [esp+y_real] ; cy
        fld st1                ; zx
        fld st1                ; zy

mandel_start:
        mov ecx,[esi+fw_iter]  ; max iterace
.man0:  fld st1
        fmul st0  ; zx^2
        fld st1
        fmul st0  ; zy^2
        fld st0
        fadd st2  ; zx^2+zy^2
;        ficomp dword [const4]
        fcomp st7
        fstsw ax
        and ah,045h
        jz .man1
        fsubp st1 ; zx^2-zy^2
        fadd st4  ; +cx
        fxch st2  ; zx
        fmul st1  ; zx*zy
        fadd st0  ; 2*zx*zy
        fadd st3  ; +cy
        fstp st1
        loop .man0
.man1:  mov eax,ecx
        finit
        pop esi
        ret

[SECTION .data]

const4  dd 4
