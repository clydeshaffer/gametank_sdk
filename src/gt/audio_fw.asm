DAC = $8000

AccBuf = $00
LFSR = $04 ;$05
Tmp = $06
FreqsH = $10
FreqsL = $18
BufferedAmplitudes = $20
WavePTR = $30
WaveStatesH = $50
WaveStatesL = $60
Inputs = $70
	.zeropage
    .repeat $20
    .byte 0
    .endrep
	.repeat $10
	.byte >Sine
	.endrep
	.repeat $90
	.byte 0
	.endrep
	.code
Amplitudes:
	.repeat 24
	.byte >Sine
	.endrep
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
.macro tickWave ch
	CLC
	LDA WaveStatesL+ch
	ADC FreqsL+ch
	STA WaveStatesL+ch
	LDA WaveStatesH+ch
	ADC FreqsH+ch
	STA WaveStatesH+ch
	BCC :+
	LDA BufferedAmplitudes+ch
	STA Amplitudes+ch
	LDA BufferedAmplitudes+ch+6
	STA Amplitudes+ch+6
	LDA BufferedAmplitudes+ch+12
	STA Amplitudes+ch+12
	LDA BufferedAmplitudes+ch+18
	STA Amplitudes+ch+18
:
.endmacro

.macro doChannel ch
	LDA WaveStatesH+ch
	STA WaveStateParams+3
	STA WaveStateParams+2
	STA WaveStateParams+1
	STA WaveStateParams+0

	LDA Amplitudes+0+ch
	STA Op1+2
	LDA Amplitudes+6+ch
	STA Op2+2
	LDA Amplitudes+12+ch
	STA Op3+2
	LDA Amplitudes+18+ch
	STA Op4+2
	JSR FMChannel
.endmacro

	tickWave 0
	tickWave 1
	tickWave 2
	tickWave 3
	;tickWave 4
	;tickWave 5
	
	doChannel 0

;channel 1 bass
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

	LDA Amplitudes+1
	STA Op1+2
	LDA Amplitudes+7
	STA Op2+2
	LDA Amplitudes+13
	STA Op3+2
	LDA Amplitudes+19
	STA Op4+2
	JSR FMChannel
;channel 1 end

;channel 2 percussion
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

	LDA Amplitudes+2
	STA Op1+2
	LDA Amplitudes+8
	STA Op2+2
	LDA Amplitudes+14
	STA Op3+2
	LDA Amplitudes+20
	STA Op4+2
	JSR FMChannel
;channel 2 end

	doChannel 3

	LDA AccBuf
	CLC
	ADC #$80
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
	SEC
	SBC #$80
	CLC
	ADC AccBuf
	STA AccBuf
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

	.align 256
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