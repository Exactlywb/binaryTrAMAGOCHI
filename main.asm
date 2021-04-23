global _start

section .text

jmp _start

;===================================================
;====================JMP TABLE======================
_callHlt:
        jmp             _hlt
_callIn:
        jmp             _in
_callOut:
        jmp             _out
;===================================================

_hlt:
        mov             rax, 3Ch        ;exit code
        xor             rdi, rdi        ;error code
        syscall

;===================================================
;Entry: nothing
;Destr: dohera
;Ret  : rax <- the number
;       rdx <- exponent: rax / 10^(rdi) = yourNumber
;===================================================
_in:
        mov             r12, rsp

        sub             rsp, 64             ;size for buffer

        mov             rax, 0h             ;
        mov             rdi, 0h             ;
        mov             rsi, rsp            ;INPUT
        mov             rdx, 64             ;
        syscall                     ;

        mov             rsp, r12

        mov             rcx, rax        
        dec             rcx                 ;length in rcx
        mov             rdi, 0              ;an amount of numbers before the dot
        call            _transformNumber    ;transform our number

        ;now we got rax with our number
        ;and rdx <- the degree of ten
        ;we have to do rax / 10^(rdx)
        ;let's f*ckin' do it

        call            pow10               ;now in rbx we have 10^(rdx)

        cvtsi2sd        xmm7, rax           ;xmm7 = rax
        cvtsi2sd        xmm6, rbx           ;xmm6 = rbx = 10^(rdx)

        divsd           xmm7, xmm6

        pop             r12

        sub             rsp, 8
        movsd           qword [rsp], xmm7   ;push this shit into stack

        push            r12

        ;if you read to this moment you must know that I'm exhausted. 20.04.2021

        ret

;===================================================
;Entry: rdx
;Destr: rbx, rdx
;Ret  : rbx = 10^(rdx)
;===================================================
pow10:
        mov             rbx, 1          ;our answer

pow10Cicle:
        cmp             rdx, 0
        je              pow10Ret

        imul            rbx, 10

        dec             rdx
        jmp             pow10Cicle

pow10Ret:
        ret

;===================================================
;Entry: rsi - pointer on input
;       rcx - length of input
;Destr: dohera
;
;Ret  : rax <- the number
;       rdx <- exponent: rax / 10^(rdi) = yourNumber
;===================================================
_transformNumber:
        xor             rax, rax            ;our number
        xor             r10, r10            ;minus will be here.

        mov             rdi, rsi            ;
        add             rdi, rcx            ;pointer on the end of the number
        dec             rdi                 ;

        xor             rdx, rdx            ;exponent

_transformCicle:
        mov             r11, rcx

        xor             rbx, rbx            ;current symbol will be in bl
        mov             bl , BYTE [rsi]     ;current symbol here

_checkMinus:
        cmp             bl , '-'
        jne             _checkDot

_tagMinus:
        mov             r10, 1
        inc             rsi
        mov             rcx, r11
        loop            _transformCicle

_checkDot:
        cmp             bl , '.'
        jne             _normalNumber

_tagDot:
        sub             rdi, rsi
        mov             rdx, rdi

        inc             rsi

        mov             rcx, r11
        loop            _transformCicle

_normalNumber:
        sub             bl , '0'            ;to get number
        imul            rax, 10             ;rax *= 10
        add             rax, rbx            ;rax += bl

        inc             rsi
        mov             rcx, r11
        loop            _transformCicle

        cmp             r10, 1
        jne             _transformRet

        xor             rax, 0xFFFFFFFFFFFFFFFF

_transformRet:
        ret

;===================================================
;Entry: number in stack
;Destr: dohera
;Ret  : rax <- the number
;       rdx <- exponent: rax / 10^(rdi) = yourNumber
;===================================================
_out:        
        pop             r11                     ;return adress
                
        movsd           qword xmm7, [rsp]
        mov             r10, 100000
        cvtsi2sd        xmm6, r10
        mulsd           xmm7, xmm6
        cvttsd2si       rbx, xmm7

        push            r11
        
        cmp             rbx, 0xFFFFFFFF
        jg              _regularNumber
        xor             rbx, 0xFFFFFFFFFFFFFFFF
        inc             rbx
        
        mov             rsi, rsp
        sub             rsi, 60                 ;
        mov             BYTE [rsi], "-"         ; print(-)
        call            _outSymb                ;

_regularNumber:
        sub             rsp, 64

        mov             rdi, rsp                ; for stosb
        mov             rsi, 10                 ; for div
        mov             rax, rbx                ; main number
        mov             r10, 0                  ; for dot

_nextHandling:
    
        xor             rdx, rdx         
        div             rsi                     ; rax /= 10
        mov             rcx, rax                ; save rax
                        
        mov             al, dl
        add             al, 0x30                ; + '0'
        stosb
        mov             rax, rcx
                
        or              rax, rax
        je              _endHandling
        jmp             _nextHandling

_endHandling:            

        dec             rdi                     ; print
        mov             rsi, rdi
        
        call            _printNumber
        
        add             rsp, 64
        jmp             _outRet

newLine: db 0xA

_outRet:

        mov             rsi, newLine
        call            _outSymb
        ret
        

;====================================================
; Print number in stdout from stack
;
; Expect: rsi - address of the symbol
; Destroy: rax, rdi, rdx 
;====================================================
_outSymb:
        mov             rax, 1      ; write
        mov             rdi, 1         ; stdout
        mov             rdx, 1         ; length
        syscall
                        
        ret

;====================================================
; Print number from stack with dot
;
; Expect: 
;     rsi - address of the first symbol
;     rsp - address ot the last symbol
; Destroy: rsi, r10, 
;====================================================
_printNumber:
        pop             r12

        sub             rsi, 5          
        cmp             rsi, rsp        
        jge             _startPrint
        add             rsi, 6          
        mov             BYTE [rsi], '0'
        call            _outSymb
        mov             BYTE [rsi], '.'
        call            _outSymb           
        dec             rsi             
        
        sub             rsi, 5          
        mov             r10, rsp        
        sub             r10, rsi        
        add             rsi, 6          
        mov             BYTE [rsi], '0' 

_fillZero:              
        cmp             r10, 1          
        jle             _endFill       
        call            _outSymb           
        dec             r10             
        jmp             _fillZero 

_endFill:
        sub             rsi, 6

_startPrint:
        add             rsi, 5

_printLoop:
        call            _outSymb
    
        sub             rsi, 5                  ; 
        cmp             rsi, rsp                ;
        jne             _printNoDot             ; If dot:
        add             rsi, 5                  ; print(.)
        mov             BYTE [rsi], "."         ;
        call            _outSymb                ;
        sub             rsi, 5                  ;

_printNoDot:
        add             rsi, 4

        cmp             rsi, rsp
        jge             _printLoop 
        
        push            r12
        ret

_start:

        call            _callIn
        call            _callOut
        call            _callOut
                
        call            _callHlt
