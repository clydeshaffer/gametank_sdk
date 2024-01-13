DAC = $8000

AccBuf = $00
LFSR = $04 ;$05
Tmp = $06
FreqsH = $10
FreqsL = $14
Amplitudes = $18
Bends = $28
WavePTR = $30
WaveStatesH = $50
WaveStatesL = $60
Inputs = $70

	.zeropage
	.byte 0, 0, 0, 0, 2, 0
    .repeat 74
    .byte 0
    .endrep

	.byte $FF, $FF, $FF, $FF

	.code
AmpParam:
	.byte 7
WaveStateParams:
	.byte 0, 0, 0, 0, 0
ScratchPad:
	.byte 0
RESET:
	CLI
    LDA #<Sine
    STA WavePTR
    LDA #>Sine
    STA WavePTR+1
Forever:
    WAI
	JMP Forever

IRQ:
	;Clear sum buffer
	STZ AccBuf ;3?
	;Update all wavestates
	CLC
	LDA WaveStatesL+0
	ADC FreqsL+0
	STA WaveStatesL+0
	LDA WaveStatesH+0
	ADC FreqsH+0
	STA WaveStatesH+0

	CLC
	LDA WaveStatesL+1
	ADC FreqsL+1
	STA WaveStatesL+1
	LDA WaveStatesH+1
	ADC FreqsH+1
	STA WaveStatesH+1

	CLC
	LDA WaveStatesL+2
	ADC FreqsL+2
	STA WaveStatesL+2
	LDA WaveStatesH+2
	ADC FreqsH+2
	STA WaveStatesH+2

	CLC
	LDA WaveStatesL+3
	ADC FreqsL+3
	STA WaveStatesL+3
	LDA WaveStatesH+3
	ADC FreqsH+3
	STA WaveStatesH+3


	LDA WaveStatesH+0
	STA WaveStateParams+3
	STA WaveStateParams+2
	STA WaveStateParams+1
	STA WaveStateParams+0

	;Channel 1 wavestate
	LDA Amplitudes+0
	STA Op1+2
	LDA Amplitudes+4
	STA Op2+2
	LDA Amplitudes+8
	STA Op3+2
	LDA Amplitudes+12
	STA Op4+2
	JSR FMChannel

	LDA WaveStatesH+1
	STA WaveStateParams+3
	STA WaveStateParams+2
	STA WaveStateParams+1
	CLC
	LDA WaveStatesL+1
	ASL
	STA ScratchPad
	LDA WaveStatesH+1
	ASL
	STA WaveStateParams+0

	LDA ScratchPad
	ASL
	STA ScratchPad
	LDA WaveStateParams+0
	ASL
	STA WaveStateParams+0

	LDA ScratchPad
	ASL
	STA ScratchPad
	LDA WaveStateParams+0
	ASL
	STA WaveStateParams+0

	;Channel 2 wavestate
	LDA Amplitudes+1
	STA Op1+2
	LDA Amplitudes+5
	STA Op2+2
	LDA Amplitudes+9
	STA Op3+2
	LDA Amplitudes+13
	STA Op4+2
	JSR FMChannel

	LDA WaveStatesH+2
	STA WaveStateParams+3
	STA WaveStateParams+2
	STA WaveStateParams+1
	CLC
	LDA WaveStatesL+2
	ASL
	STA ScratchPad
	LDA WaveStatesH+2
	ASL
	STA WaveStateParams+0

	LDA ScratchPad
	ASL
	STA ScratchPad
	LDA WaveStateParams+0
	ASL
	STA WaveStateParams+0

	LDA ScratchPad
	ASL
	STA ScratchPad
	LDA WaveStateParams+0
	ASL
	STA WaveStateParams+0

	;Channel 3 wavestate
	LDA Amplitudes+2
	STA Op1+2
	LDA Amplitudes+6
	STA Op2+2
	LDA Amplitudes+10
	STA Op3+2
	LDA Amplitudes+14
	STA Op4+2
	JSR FMChannel

	LDA WaveStatesH+3
	STA WaveStateParams+3
	STA WaveStateParams+2
	STA WaveStateParams+1
	STA WaveStateParams+0

	;Channel 4 wavestate
	LDA Amplitudes+3
	STA Op1+2
	LDA Amplitudes+7
	STA Op2+2
	LDA Amplitudes+11
	STA Op3+2
	LDA Amplitudes+15
	STA Op4+2
	JSR FMChannel

	LDA AccBuf
	STA DAC

	RTI ;6

FMChannel:
	CLC
	LDA WaveStateParams+0
	TAX
Op1:
	LDA Sine, x
	CLC
	ADC WaveStateParams+1
	TAX
Op2:
	LDA Sine, x
	CLC
	ADC WaveStateParams+2
	TAX

Op3:
	LDA Sine, x
	CLC
	ADC WaveStateParams+3
	TAX

Op4:
	LDA Sine, x
	CLC
	ADC AccBuf
	STA AccBuf ;3
	RTS

;Read inputs addr, val until addr=0
NMI_handler:
    PHY
    PHX
    PHA
    LDY #0
NMI_Loop:
    LDX Inputs, y
    BEQ NMI_Done
    INY
    LDA Inputs, y
    STA $00, x
    INY
    JMP NMI_Loop
    
NMI_Done:
    PLA
    PLX
    PLY
	RTI

	.align 8
Sine:
	.incbin "sine.raw"
	.incbin "sine.raw"
	.incbin "sine.raw"
	.incbin "sine.raw"
	.incbin "sine.raw"
	.incbin "sine.raw"
	.incbin "sine.raw"
	.incbin "sine.raw"

	.segment "VECTORS"
	.addr NMI_handler
	.addr RESET
	.addr IRQ