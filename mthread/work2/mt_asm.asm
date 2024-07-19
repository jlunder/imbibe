.286


                PUBLIC  mt_init_
                PUBLIC  mt_done_
                PUBLIC  mt_disable_
                PUBLIC  mt_enable_
                PUBLIC  _timer_count
                PUBLIC  _hita
                PUBLIC  _hitb


DGROUP          GROUP   CONST, CONST2, _DATA, _BSS
_TEXT           SEGMENT PARA PUBLIC USE16 'CODE'
                ASSUME  CS:_TEXT, DS:DGROUP, SS:DGROUP


mt_handler_     PROC
                push    ds
                push    SEG origin
                pop     ds
                add     WORD PTR _timer_count, 1
                adc     WORD PTR _timer_count + 2, 0
                add     WORD PTR _overflow, 01234DDh / 1000
                jc      @1
                push    ax
                mov     al, 20h
                out     20h, al
                pop     ax
                pop     ds
                iret
@1:             pushf
                callf   DWORD PTR _mt_bios_handler
                pop     ds
                iret
mt_handler_     ENDP


mt_init_        PROC
                push    ax
                push    bx
                push    cx
                push    dx
                push    es
                mov     ah, 35h
                mov     al, 08h
                int     21h
                mov     WORD PTR _mt_bios_handler, bx
                mov     WORD PTR _mt_bios_handler + 2, es
                mov     WORD PTR _timer_count, 0
                mov     WORD PTR _timer_count + 2, 0
                mov     WORD PTR _overflow, 0
                pop     es
                pushf
                cli
                mov     al, 034h
                out     043h, al
                mov     al, (01234DDh / 1000) and 0FFh
                out     040h, al
                mov     al, (01234DDh / 1000) / 256
                out     040h, al
                mov     ah, 25h
                mov     al, 08h
                push    ds
                push    SEG timer_handler_
                pop     ds
                mov     dx, OFFSET timer_handler_
                int     21h
                pop     ds
                popf
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                ret
mt_init_        ENDP


mt_done_        PROC
                push    ax
                push    bx
                push    cx
                push    dx
                pushf
                cli
                mov     al, 034h
                out     043h, al
                mov     al, 0
                out     040h, al
                out     040h, al
                push    ds
                mov     ah, 25h
                mov     al, 08h
                lds     dx, DWORD PTR _mt_bios_handler
                int     21h
                pop     ds
                popf
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                ret
mt_done_        ENDP


mt_disable_     PROC
                pushf
                cli
                inc     mt_lock_
                popf
                ret
mt_disable_     ENDP


mt_enable_      PROC
                pushf
                cli
                dec     mt_lock_
                popf
                ret
mt_enable_      ENDP


_TEXT           ENDS


CONST           SEGMENT DWORD PUBLIC USE16 'DATA'

                ORG 0h
origin          LABEL BYTE

CONST           ENDS


CONST2          SEGMENT DWORD PUBLIC USE16 'DATA'
CONST2          ENDS


_DATA           SEGMENT DWORD PUBLIC USE16 'DATA'
_DATA           ENDS


_BSS            SEGMENT DWORD PUBLIC USE16 'BSS'

                ORG     0h
_mt_timer_count DD      ?
_mt_bios_handler
                DD      ?
_overflow       DW      ?
_mt_lock        DW      ?
_BSS            ENDS


END
