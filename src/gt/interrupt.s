; ---------------------------------------------------------------------------
; interrupt.s
; ---------------------------------------------------------------------------
;
; Interrupt handler.
;
; Checks for a BRK instruction and returns from all valid interrupts.

.include "../gen/modules_enabled.inc"

.import   _frameflag
.import   _draw_busy
.export   _irq_int, _nmi_int

.ifdef ENABLE_MODULE_DRAWQUEUE
.import _next_draw_queue, _queue_end, _queue_pending, _queue_start
.endif

.pc02

DMA_Start = $4006

.segment  "CODE"

.PC02                             ; Force 65C02 assembly mode

; ---------------------------------------------------------------------------
; Non-maskable interrupt (NMI) service routine

_nmi_int:
        PHA
        LDA $1FFF
        BNE nmi_done
        STZ _frameflag
nmi_done:
        PLA
        RTI

; ---------------------------------------------------------------------------
; Maskable interrupt (IRQ) service routine

.ifdef ENABLE_MODULE_DRAWQUEUE
_irq_int:
        PHX                    ; Save X register contents to stack
        PHA
        PHY

        ;make sure DMA isn't running then compare head and tail of queue
        ;to determine whether there is more to process
        STZ DMA_Start
        STZ _queue_pending
        STZ _draw_busy
        LDA _queue_start
        CMP _queue_end
        BEQ finish_irq
        JSR _next_draw_queue
finish_irq:
        PLY
        PLA                    ; Restore accumulator contents
        PLX                    ; Restore X register contents
        RTI                    ; Return from all IRQ interrupts
.else
_irq_int:
        STZ DMA_Start
        STZ _draw_busy
        RTI
.endif