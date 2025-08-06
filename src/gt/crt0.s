; ---------------------------------------------------------------------------
; crt0.s
; ---------------------------------------------------------------------------
;
; Startup code for cc65 (GameTank version)

.export   _init, _exit
.import   _sdk_init, _main

.export   __STARTUP__ : absolute = 1        ; Mark as startup
.import   __RAM_START__, __RAM_SIZE__       ; Linker generated

.import    copydata, zerobss, initlib, donelib

.import _romBankMirror
.import _bankflip
.export _SwitchRomBank

.export _DynaWave
.export _SineaWave

.PC02

BankReg = $2005
VIA = $2800
ORB = 0
ORAr = 1
DDRB = 2
DDRA = 3
T1C = 5
ACR = $B
PCR = $C
IFR = $D
IER = $E

.include  "zeropage.inc"

; ---------------------------------------------------------------------------
; Place the startup code in a special segment

.segment  "STARTUP"

; ---------------------------------------------------------------------------
; A little light 6502 housekeeping

_init:    LDX     #$FF                 ; Initialize stack pointer to $01FF
          TXS
          CLD                          ; Clear decimal mode

    ldx #0
viaWakeup:
	inx
	bne viaWakeup

	LDA #127
	STA VIA+IER ;disable VIA IRQs
	
	LDA #40
	STA BankReg
	STA $1FFF
	STZ BankReg
	STZ $1FFF

	LDA #%00000111
	STA VIA+DDRA
    LDA #$FF
    STA VIA+ORAr
	lda #$80
	sta _romBankMirror
	jsr ShiftHighBits

; ---------------------------------------------------------------------------
; Set cc65 argument stack pointer

          LDA     #<(__RAM_START__ + __RAM_SIZE__)
          STA     sp
          LDA     #>(__RAM_START__ + __RAM_SIZE__)
          STA     sp+1

; ---------------------------------------------------------------------------
; Initialize memory storage

          JSR     zerobss              ; Clear BSS segment
          JSR     copydata             ; Initialize DATA segment
          JSR     initlib              ; Run constructors

		  STZ _bankflip

; ---------------------------------------------------------------------------
; Call main()

          JSR     _sdk_init
		  JSR     _main

; ---------------------------------------------------------------------------
; Back from main (this is also the _exit entry):  force a software break

_exit:    JSR     donelib              ; Run destructors
          BRK

.proc _SwitchRomBank: near
	PHP
	SEI
	JSR ShiftHighBits
	PLP
	RTS
.endproc

ShiftHighBits:
	STA $0
	LDA #$FF
	STA VIA+ORAr

	LDA $0
	ROR
	ROR
	ROR
	ROR
	ROR
	ROR
	AND #2
	ORA #%00000100
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr

	LDA $0
	ROR
	ROR
	ROR
	ROR
	ROR
	AND #2
	ORA #%00000100
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr

	LDA $0
	ROR
	ROR
	ROR
	ROR
	AND #2
	ORA #%00000100
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr

	LDA $0
	ROR
	ROR
	ROR
	AND #2
	ORA #%00000100
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr

	LDA $0
	ROR
	ROR
	AND #2
	ORA #%00000100
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr
	
	LDA $0
	ROR
	AND #2
	ORA #%00000100
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr

	LDA $0
	AND #2
	ORA #%00000100
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr

	LDA $0
	ROL
	AND #2
	STA VIA+ORAr
	ORA #1
	STA VIA+ORAr

	ORA #4
	STA VIA+ORAr

	RTS

	.segment "COMMON"
_DynaWave:
	.incbin "build/assets/audio_fw_classic.bin.deflate"
_SineaWave:
    .incbin "build/assets/audio_fw.bin.deflate"