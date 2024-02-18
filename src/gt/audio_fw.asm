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

.macro tickChannel ch
	tickWave (ch * 4)

	BCC :+
	LDA BufferedAmplitudes+(ch*4)+0
	STA Amplitudes+(ch*4)+0
	LDA BufferedAmplitudes+(ch*4)+1
	STA Amplitudes+(ch*4)+1
	LDA BufferedAmplitudes+(ch*4)+2
	STA Amplitudes+(ch*4)+2
	LDA BufferedAmplitudes+(ch*4)+3
	STA Amplitudes+(ch*4)+3
:
	tickWave (ch*4)+1
	tickWave (ch*4)+2
	tickWave (ch*4)+3
.endmacro

.macro doChannel ch
	LDA WaveStatesH+(ch*4)+3
	STA Op4Param+1
	LDA WaveStatesH+(ch*4)+2
	STA Op3Param+1
	LDA WaveStatesH+(ch*4)+1
	STA Op2Param+1
	;CLC
	LDA WaveStatesH+(ch*4)+0
	ADC LastSample+ch
	SEC
	SBC #$80
	STA Op1Param+1

	LDA Amplitudes+(ch*4)+0
	STA Op1+2
	LDA Amplitudes+(ch*4)+1
	STA Op2+2
	LDA Amplitudes+(ch*4)+2
	STA Op3+2
	LDA Amplitudes+(ch*4)+3
	STA Op4+2
	LDA #LastSample+ch
	STA SaveFeedback+1
	LDA FeedbackAmount+ch
	STA SampleFeedback+2
	JSR FMChannel
.endmacro

IRQ:
	;Clear sum buffer
	STZ AccBuf

	;Update all wavestates
	tickChannel 0
	tickChannel 1
	tickChannel 2
	tickChannel 3

	doChannel 0
	doChannel 1
	doChannel 2
	doChannel 3

	LDA AccBuf
	CLC
	ADC #$80
	STA DAC

	RTI ;6

FMChannel:
	CLC
Op1Param:
	LDA #0
	TAX
SampleFeedback:	
	LDA Sine, x
SaveFeedback:
	STA LastSample+0
Op1:
	LDA Sine, x
	CLC
Op2Param:
	ADC #0
	TAX
Op2:
	LDA Sine, x
	CLC
	BRA Op3Param ;;cutoff point betweeb single 4-op and dual 2-op FM
:
	ADC AccBuf
	STA AccBuf
	CLC
	LDA #0
Op3Param:
	ADC #0
	TAX

Op3:
	LDA Sine, x
	CLC
Op4Param:
	ADC #0
	TAX

Op4:
	LDA Sine, x
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
	.incbin "scaled_sines.raw"

	.segment "VECTORS"
	.addr NMI_handler
	.addr RESET
	.addr IRQ