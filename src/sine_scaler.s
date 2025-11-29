.export _get_scaled_sine, _Sine256

.zeropage
SineTheta:
.res 1
SineOffset:
.res 1
SineAcc:
.res 1
.code

_Sine256:
    .byte $00,$03,$06,$09,$0C,$10,$13,$16
    .byte $19,$1C,$1F,$22,$25,$28,$2B,$2E
    .byte $31,$33,$36,$39,$3C,$3F,$41,$44
    .byte $47,$49,$4C,$4E,$51,$53,$55,$58
    .byte $5A,$5C,$5E,$60,$62,$64,$66,$68
    .byte $6A,$6B,$6D,$6F,$70,$71,$73,$74
    .byte $75,$76,$78,$79,$7A,$7A,$7B,$7C
    .byte $7D,$7D,$7E,$7E,$7E,$7F,$7F,$7F
    .byte $7F,$7F,$7F,$7F,$7E,$7E,$7E,$7D
    .byte $7D,$7C,$7B,$7A,$7A,$79,$78,$76
    .byte $75,$74,$73,$71,$70,$6F,$6D,$6B
    .byte $6A,$68,$66,$64,$62,$60,$5E,$5C
    .byte $5A,$58,$55,$53,$51,$4E,$4C,$49
    .byte $47,$44,$41,$3F,$3C,$39,$36,$33
    .byte $31,$2E,$2B,$28,$25,$22,$1F,$1C
    .byte $19,$16,$13,$10,$0C,$09,$06,$03
    .byte $00,$FD,$FA,$F7,$F4,$F0,$ED,$EA
    .byte $E7,$E4,$E1,$DE,$DB,$D8,$D5,$D2
    .byte $CF,$CD,$CA,$C7,$C4,$C1,$BF,$BC
    .byte $B9,$B7,$B4,$B2,$AF,$AD,$AB,$A8
    .byte $A6,$A4,$A2,$A0,$9E,$9C,$9A,$98
    .byte $96,$95,$93,$91,$90,$8F,$8D,$8C
    .byte $8B,$8A,$88,$87,$86,$86,$85,$84
    .byte $83,$83,$82,$82,$82,$81,$81,$81
    .byte $81,$81,$81,$81,$82,$82,$82,$83
    .byte $83,$84,$85,$86,$86,$87,$88,$8A
    .byte $8B,$8C,$8D,$8F,$90,$91,$93,$95
    .byte $96,$98,$9A,$9C,$9E,$A0,$A2,$A4
    .byte $A6,$A8,$AB,$AD,$AF,$B2,$B4,$B7
    .byte $B9,$BC,$BF,$C1,$C4,$C7,$CA,$CD
    .byte $CF,$D2,$D5,$D8,$DB,$DE,$E1,$E4
    .byte $E7,$EA,$ED,$F0,$F4,$F7,$FA,$FD

.proc _get_scaled_sine: near
    STA SineTheta
    STX SineOffset

    CLC
    ADC SineTheta
    TAX
    LDA _Sine256, x
    STA SineAcc

    LDA SineTheta
    SEC
    SBC SineOffset
    TAX
    LDA _Sine256, x
    
    CLC
    ADC SineAcc
    LDX #0
    RTS
.endproc