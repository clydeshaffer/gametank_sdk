; ---------------------------------------------------------------------------
; draw_logo.s
; ---------------------------------------------------------------------------
;
; Draw the GameTank logo

.export  _draw_gametank_logo

DMA_VX = $4000
DMA_VY = $4001
DMA_GX = $4002
DMA_GY = $4003
DMA_WIDTH = $4004
DMA_HEIGHT = $4005
DMA_Start = $4006
DMA_Color = $4007

.segment  "COMMON"

;includes labels LogoXVals and LogoWVals
.include "gametank_logo.inc"

.proc _draw_gametank_logo: near
        PHY
        LDY #1
        STY DMA_HEIGHT
        STA DMA_Color
        LDX #0
        LDY #56
DrawLoop:
        LDA LogoWVals, x
DrawLoopSkipW:
        BEQ NextLine
        STA DMA_WIDTH
        LDA LogoXVals, x
        STA DMA_VX
        LDA #1
        STA DMA_Start
        INX
        BRA DrawLoop
NextLine:
        INY
        STY DMA_VY
        INX
        LDA LogoWVals, x
        BEQ EndLoop
        BRA DrawLoopSkipW
EndLoop:

        SEI
        CLI
        PLY
        RTS                    ; Return to caller

.endproc