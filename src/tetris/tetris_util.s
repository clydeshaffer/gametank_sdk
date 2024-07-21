.export _add_tmp_score_bcd
.import _tmpscore

.pc02
.CODE

.proc _add_tmp_score_bcd: near

    SED
    CLC
    ADC _tmpscore
    STA _tmpscore
    LDA #0
    ADC _tmpscore+1
    STA _tmpscore+1
    CLD
    RTS

.endproc