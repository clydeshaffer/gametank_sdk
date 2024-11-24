; ---------------------------------------------------------------------------
; crt0.s
; ---------------------------------------------------------------------------
;
; Startup code for cc65 (GameTank version)

.export   _init, _exit
.import   _main, _sdk_init

.export   __STARTUP__ : absolute = 1        ; Mark as startup
.import   __RAM_START__, __RAM_SIZE__       ; Linker generated

.import    copydata, zerobss, initlib, donelib

.import _romBankMirror
.import _bankflip
.import _bank_shift_out

.export _AudioFWPkg

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

; ----------------------------------------------------------<.BANK (_sdk_init)----------------
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
	
	LDA #40
	STA BankReg
	STA $1FFF
	STZ BankReg
	STZ $1FFF

	LDA #%00000111
	STA VIA+DDRA
    LDA #$FF
    STA VIA+ORAr
	LDA #$FD
	jsr _bank_shift_out


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

		  LDA #$FD
		  jsr _bank_shift_out

		  JSR	  _sdk_init
          JSR     _main

; ---------------------------------------------------------------------------
; Back from main (this is also the _exit entry):  force a software break

_exit:    JSR     donelib              ; Run destructors
          BRK

	.segment "COMMON"
_AudioFWPkg:
    .incbin "build/assets/audio_fw.bin.deflate"