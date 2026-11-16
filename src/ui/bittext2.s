.export _bittext_x, _bittext_draw_char
.export _bittext_set_x, _bittext_set_y
.import _system_font_monochrome, _bittext_color, _bittext_bg
.export text_row_0

.segment "ZEROPAGE"
bittext_pat:
.byte 0, 0

.segment "BSS"
_bittext_x:
    .byte 0

.segment "DATA"
_bittext_draw_char:
    ;A should be char
    SEC
    SBC #32 ;space character
    ASL
    STA bittext_pat
    LDA #0
    ROL
    STA bittext_pat+1
    LDA bittext_pat
    ASL
    STA bittext_pat
    LDA bittext_pat+1
    ROL
    STA bittext_pat+1
    CLC
    LDA loadFontPat+1
    ADC bittext_pat
    STA loadFontPat+1
    LDA loadFontPat+2
    ADC bittext_pat+1
    STA loadFontPat+2
    LDX #0

    LDY _bittext_x
;LoaD from system_font_monochrome ;;this gets added to by previous instruction and restored at end
loadFontPat:
    LDA _system_font_monochrome, x
    STA bittext_pat
    

    LDA _bittext_bg
    BBR0 bittext_pat, :+
    LDA _bittext_color
:
text_row_0:
    STA $4000, y

    LDA _bittext_bg
    BBR1 bittext_pat, :+
    LDA _bittext_color
:
text_row_1:
    STA $4080, y

    LDA _bittext_bg
    BBR2 bittext_pat, :+
    LDA _bittext_color
:
text_row_2:
    STA $4000, y

    LDA _bittext_bg
    BBR3 bittext_pat, :+
    LDA _bittext_color
:
text_row_3:
    STA $4080, y

    LDA _bittext_bg
    BBR4 bittext_pat, :+
    LDA _bittext_color
:
text_row_4:
    STA $4000, y

    LDA _bittext_bg
    BBR5 bittext_pat, :+
    LDA _bittext_color
:
text_row_5:
    STA $4080, y

    LDA _bittext_bg
    BBR6 bittext_pat, :+
    LDA _bittext_color
:
text_row_6:
    STA $4000, y

    LDA _bittext_bg
    BBR7 bittext_pat, :+
    LDA _bittext_color
:
text_row_7:
    STA $4080, y

    INY
    INX
    CPX #4
    BEQ textdone
    JMP loadFontPat
textdone:
    STY _bittext_x

    LDA #<_system_font_monochrome
    STA loadFontPat+1
    LDA #>_system_font_monochrome
    STA loadFontPat+2
    RTS
    
.segment "CODE"
_bittext_set_x:
    TAY
    LDA _bittext_x
    AND #$80
    STA _bittext_x
    TYA
    ORA _bittext_x
    STA _bittext_x
    RTS

_bittext_set_y:
    TAY
    LSR
    CLC
    ADC #$40
    STA text_row_0+2
    STA text_row_1+2
    INC
    STA text_row_2+2
    STA text_row_3+2
    INC
    STA text_row_4+2
    STA text_row_5+2
    INC
    STA text_row_6+2
    STA text_row_7+2
    LDA _bittext_x
    AND #$7F
    STA _bittext_x
    TYA
    ROR
    ROR
    AND #$80
    ORA _bittext_x
    STA _bittext_x
    RTS