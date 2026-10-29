.include "../../../../gen/modules_enabled.inc"

.ifdef ENABLE_MODULE_SDK_TRACKER_FM_BACKEND

.import _channelStates, _setEnvChannel, _setEnvInst, _env_ptr_cache, _music_channel_mask
.export _setEnvelopeFetchers, _fetchEnvelopes
.export ch0op3, ch1op3

.zeropage
instrPtr:
.byte 0, 0
fetcherPtr:
.byte 0, 0

.bss
_instrument_table:
.byte 0, 0

.segment "DATA"
_fetchEnvelopes:
ch0:
	lda _music_channel_mask
	and #1
	beq ch1
	ldy _channelStates+0
	bmi ch1
	ch0op0:
	lda $8000, y
	sta $3030
	ch0op1:
	lda $8000, y
	sta $3031
	ch0op2:
	lda $8000, y
	sta $3032
	ch0op3:
	lda $8000, y
	sta $3033
	
ch1:
	lda _music_channel_mask
	and #2
	beq ch2
	ldy _channelStates+1
	bmi ch2
	ch1op0:
	lda $8000, y
	sta $3034
	ch1op1:
	lda $8000, y
	sta $3035
	ch1op2:
	lda $8000, y
	sta $3036
	ch1op3:
	lda $8000, y
	sta $3037
	
ch2:
	lda _music_channel_mask
	and #4
	beq ch3
	ldy _channelStates+2
	bmi ch3
	ch2op0:
	lda $8000, y
	sta $3038
	ch2op1:
	lda $8000, y
	sta $3039
	ch2op2:
	lda $8000, y
	sta $303A
	ch2op3:
	lda $8000, y
	sta $303B

ch3:
	lda _music_channel_mask
	and #8
	beq fetchEnd
	ldy _channelStates+3
	bmi fetchEnd
	ch3op0:
	lda $8000, y
	sta $303C
	ch3op1:
	lda $8000, y
	sta $303D
	ch3op2:
	lda $8000, y
	sta $303E
	ch3op3:
	lda $8000, y
	sta $303F
	
fetchEnd:
	rts


.segment "CODE"

channelFetchersLSB:
.byte <ch0op0, <ch1op0, <ch2op0, <ch3op0
channelFetchersMSB:
.byte >ch0op0, >ch1op0, >ch2op0, >ch3op0

.macro incY5
	TYA
	CLC
	ADC #05
	TAY
.endmacro
.macro setEnvPtr
	lda _env_ptr_cache, x
	sta (fetcherPtr), y
	inx
	iny
	lda _env_ptr_cache, x
	sta (fetcherPtr), y
	inx
	incY5
.endmacro

;A channel index X instrument index
_setEnvelopeFetchers:
	lda _setEnvChannel
	ldx _setEnvInst

	tay
	lda channelFetchersLSB, y
	sta fetcherPtr
	lda channelFetchersMSB, y
	sta fetcherPtr+1
	;fetcherPtr now points to the first instruction to modify

	ldy #1

	txa
	asl
	asl
	asl
	tax

	setEnvPtr
	setEnvPtr
	setEnvPtr
	setEnvPtr



	rts
	
.endif