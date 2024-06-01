.export _bank_shift_out
.import _romBankMirror

; ---------------------------------------------------------------
; void __near__ bank_shift_out ()
; ---------------------------------------------------------------

OutBits = $2801

.segment	"CODE"

.proc	_bank_shift_out: near
    sta _romBankMirror;
    stz OutBits
    clc
    rol
    rol
    rol
    tay

.repeat 7
    and #2
    sta OutBits
    ora #1
    sta OutBits
    tya
    rol
    tay
.endrepeat

    and #2
    sta OutBits
    ora #1
    sta OutBits
    ora #4
    sta OutBits
    stz OutBits
    
    rts

.endproc