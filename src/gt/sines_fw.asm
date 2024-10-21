DAC = $8000
AccBuf = $00
WavePTR = $02
WavePTR_MSB = $03
FeedbackAmount = $04
;reserve $05
;reserve $06
;reserve $07
LastSample = $08
;reserve $09
;reserve $0A
;reserve $0B
FreqsH = $10
FreqsL = $20
BufferedAmplitudes = $30
WaveStatesH = $50
WaveStatesL = $60
Inputs = $70
	.zeropage
	.byte 0, 0, 0, 0, >Sine, >Sine, >Sine+8, >Sine, $80, $80, $80, $80, 0, 0, 0, 0
    .repeat $20
    .byte 0
    .endrep
	.repeat $10
	.byte >Sine
	.endrep
	.repeat $80
	.byte 0
	.endrep
	.code
Amplitudes:
	.repeat 24
	.byte >Sine
	.endrep
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

.macro tickWave chn
	CLC
	LDA WaveStatesL+chn
	ADC FreqsL+chn
	STA WaveStatesL+chn
	LDA WaveStatesH+chn
	ADC FreqsH+chn
	STA WaveStatesH+chn
.endmacro

.macro sampleWave chn
	LDX WaveStatesH+chn
	LDA Sine+1024, x
	CLC
	ADC AccBuf
	STA AccBuf
.endmacro

IRQ:
	;Clear sum buffer
	STZ AccBuf
	
	tickWave 0
	tickWave 1
	tickWave 2
	tickWave 3
	tickWave 4
	tickWave 5
	tickWave 6
	tickWave 7
	
	sampleWave 0
	sampleWave 1
	sampleWave 2
	sampleWave 3
	sampleWave 4
	sampleWave 5
	sampleWave 6
	sampleWave 7

	CLC
	LDA AccBuf
	ADC #$80
	STA DAC

	RTI ;6

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
	.incbin "scaled_sines.raw"

	.segment "VECTORS"
	.addr NMI_handler
	.addr RESET
	.addr IRQ