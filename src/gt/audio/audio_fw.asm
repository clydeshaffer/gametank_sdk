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
	;AccBuf
	.byte 0, 0
	;WavePTR
	.byte 0, 0
	;Feedback
	.byte $80, $80, $80, $80
	;UNUSED
	.repeat 8
	.byte 0
	.endrep
	;Frequencies
    .repeat $20
    .byte 0
    .endrep
	;Amplitudes
	.repeat $10
	.byte $80
	.endrep
	;Wave States
	.repeat $80
	.byte 0
	.endrep

	.code
RESET:
	CLI
    LDA #<Sine
    STA WavePTR
    LDA #>Sine
    STA WavePTR+1
Forever:
    WAI
	JMP Forever

.macro tickWave op, out
	CLC
	LDA WaveStatesL+op
	ADC FreqsL+op
	STA WaveStatesL+op
	LDA out
	ADC FreqsH+op
	STA out
.endmacro

.macro doChannel ch
.local Op1
.local Op1State
.local Op1Param
.local Op2
.local Op2Param
.local Op3
.local Op3Param
.local Op4
.local Op4Param
.local SaveFeedback
.local SampleFeedback
.local LastSample

	LDA BufferedAmplitudes+(ch*4)+0
	STA Op1+1
	LDA BufferedAmplitudes+(ch*4)+1
	STA Op2+1
	LDA BufferedAmplitudes+(ch*4)+2
	STA Op3+1
	LDA BufferedAmplitudes+(ch*4)+3
	STA Op4+1

	tickWave (ch*4)+1, Op2Param+1
	tickWave (ch*4)+2, Op3Param+1
	tickWave (ch*4)+3, Op4Param+1

	tickWave (ch * 4), Op1State+1
Op1State:
	LDA #0
LastSample:
	ADC #0
	
	TAY
	CLC
SampleFeedback:
	ADC FeedbackAmount+ch
	TAX
	CLC
	LDA Sine, y
	ADC Sine, x 

SaveFeedback:
	STA LastSample+1
	TYA

	CLC
Op1:
	ADC #0
	TAX
	CLC
	LDA Sine, y
	ADC Sine, x 

	CLC
Op2Param:
	ADC #0
	TAY
	CLC
Op2:
	ADC #0
	TAX
	CLC
	LDA Sine, y
	ADC Sine, x 

	CLC
Op3Param:
	ADC #0
	TAY
	CLC
Op3:
	ADC #0
	TAX
	CLC
	LDA Sine, y
	ADC Sine, x 

	CLC
Op4Param:
	ADC #0
	TAY
	CLC
Op4:
	ADC #0
	TAX
	CLC
	LDA Sine, y
	ADC Sine, x 

	CMP #$80
	ROR
	CMP #$80
	ROR

	CLC
	ADC AccBuf
	STA AccBuf
.endmacro

.macro GetSine
.local ScaleConstant
	; Returns sin(Acc + X) + sin(Acc - X)
	TAY
	STX ScaleConstant+1
	CLC
ScaleConstant:
	ADC #0
	TAX
	CLC
	LDA Sine, y
	ADC Sine, x 
.endmacro


IRQ:
	;Clear sum buffer
	STZ AccBuf

	doChannel 0
	doChannel 1
	doChannel 2
	doChannel 3

	LDA AccBuf
	CLC
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

	.segment "WAVES"
Sine:
	.incbin "sine_256_-63_63.bin"

	.segment "VECTORS"
	.addr NMI_handler
	.addr RESET
	.addr IRQ