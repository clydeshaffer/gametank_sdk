.export _pushRect, _rect, _queue_flags_param, _next_draw_queue
.importzp sp, ptr1
.import _queue_start,_queue_end, _queue_count, _queue_pending
.import incsp2, pushax
.import _flagsMirror, _frameflip, _draw_busy
.import _banksMirror

BankReg = $2005
DMAFlags = $2007

Q_DMAFlags = $0200
Q_VX       = $0300
Q_VY       = $0400
Q_GX       = $0500
Q_GY       = $0600
Q_WIDTH    = $0700
Q_HEIGHT   = $0800
Q_Color    = $0900
Q_BankReg  = $0A00

DMA_VX = $4000
DMA_VY = $4001
DMA_GX = $4002
DMA_GY = $4003
DMA_WIDTH = $4004
DMA_HEIGHT = $4005
DMA_Start = $4006
DMA_Color = $4007

_queue_flags_param = $3200
_rect = $3201 ;borrow unbanked Audio RAM memory

; ---------------------------------------------------------------
; void __near__ pushRect ()
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_pushRect: near

.segment	"CODE"

    jsr     pushax

    ldy     _queue_end
    inc     _queue_end
    inc     _queue_count

    lda     _banksMirror ;get current banks reg
    tax
    ora     #$40 ;make BANK1 version of same
    sta     BankReg ;switch to BANK1


    lda     _rect+7 ;bank
    sta     Q_BankReg, y

    lda     _rect+0 ;x
    sta     Q_VX, y

    lda     _rect+1 ;y
    sta     Q_VY, y

    lda     _rect+4 ;gx
    sta     Q_GX, y

    lda     _rect+5 ;gy
    sta     Q_GY, y

    lda     _rect+2 ;w
    sta     Q_WIDTH, y

    lda     _rect+3 ;h
    sta     Q_HEIGHT, y

    lda     _rect+6 ;color
    sta     Q_Color, y

    lda     _queue_flags_param ;flags
    sta     Q_DMAFlags, y

    ;now finally switch back to BANK0
    stx     BankReg

    jmp     incsp2

.endproc

.proc _next_draw_queue: near
        ;determined that there is more to process
        ;so load next set of parameters
        STZ BankReg
        LDA #1
        STA _queue_pending

        ;make sure DMA mode is set to input these params
        LDA _flagsMirror
        ORA #$01

        LDY _queue_start

        LDA #$40
        STA BankReg

        LDA Q_VX, y
        STA DMA_VX

        LDA Q_VY, y
        STA DMA_VY

        LDA Q_GX, y
        STA DMA_GX

        LDA Q_GY, y
        STA DMA_GY

        LDA Q_WIDTH, y
        STA DMA_WIDTH

        LDA Q_HEIGHT, y
        STA DMA_HEIGHT

        LDA Q_Color, y
        STA DMA_Color

        LDX Q_DMAFlags, y
        LDA Q_BankReg, y

        AND #$3F
        STA BankReg
        STA _banksMirror

        TXA ;retrieve Q_DMAFlags value
        ORA _frameflip
        ORA #$45 ; DMA_ENABLE | DMA_NMI | DMA_IRQ
        STA _flagsMirror
        STA DMAFlags

        INC _queue_start

        LDA #1
        STA _draw_busy
        STA DMA_Start ;START

        DEC _queue_count
        RTS
.endproc