.export _get_scaled_sine, _Sine256, _SineOffset, _SineTheta

.zeropage
_SineOffset:
.res 1
_SineTheta:
.res 1

.rodata

_Sine256:
    .byte $00,$02,$03,$05,$06,$08,$09,$0B
    .byte $0C,$0E,$10,$11,$13,$14,$16,$17
    .byte $18,$1A,$1B,$1D,$1E,$20,$21,$22
    .byte $24,$25,$26,$27,$29,$2A,$2B,$2C
    .byte $2D,$2E,$2F,$30,$31,$32,$33,$34
    .byte $35,$36,$37,$38,$38,$39,$3A,$3B
    .byte $3B,$3C,$3C,$3D,$3D,$3E,$3E,$3E
    .byte $3F,$3F,$3F,$40,$40,$40,$40,$40
    .byte $40,$40,$40,$40,$40,$40,$3F,$3F
    .byte $3F,$3E,$3E,$3E,$3D,$3D,$3C,$3C
    .byte $3B,$3B,$3A,$39,$38,$38,$37,$36
    .byte $35,$34,$33,$32,$31,$30,$2F,$2E
    .byte $2D,$2C,$2B,$2A,$29,$27,$26,$25
    .byte $24,$22,$21,$20,$1E,$1D,$1B,$1A
    .byte $18,$17,$16,$14,$13,$11,$10,$0E
    .byte $0C,$0B,$09,$08,$06,$05,$03,$02
    .byte $00,$FE,$FD,$FB,$FA,$F8,$F7,$F5
    .byte $F4,$F2,$F0,$EF,$ED,$EC,$EA,$E9
    .byte $E8,$E6,$E5,$E3,$E2,$E0,$DF,$DE
    .byte $DC,$DB,$DA,$D9,$D7,$D6,$D5,$D4
    .byte $D3,$D2,$D1,$D0,$CF,$CE,$CD,$CC
    .byte $CB,$CA,$C9,$C8,$C8,$C7,$C6,$C5
    .byte $C5,$C4,$C4,$C3,$C3,$C2,$C2,$C2
    .byte $C1,$C1,$C1,$C0,$C0,$C0,$C0,$C0
    .byte $C0,$C0,$C0,$C0,$C0,$C0,$C1,$C1
    .byte $C1,$C2,$C2,$C2,$C3,$C3,$C4,$C4
    .byte $C5,$C5,$C6,$C7,$C8,$C8,$C9,$CA
    .byte $CB,$CC,$CD,$CE,$CF,$D0,$D1,$D2
    .byte $D3,$D4,$D5,$D6,$D7,$D9,$DA,$DB
    .byte $DC,$DE,$DF,$E0,$E2,$E3,$E5,$E6
    .byte $E8,$E9,$EA,$EC,$ED,$EF,$F0,$F2
    .byte $F4,$F5,$F7,$F8,$FA,$FB,$FD,$FE
.code

.proc _get_scaled_sine: near
    CLC
    LDA _SineTheta
    ADC _SineOffset
    TAX
    SEC
    LDA _SineTheta
    SBC _SineOffset
    TAY
    CLC
    LDA _Sine256, x
    ADC _Sine256, y
    LDX #0
    RTS
.endproc