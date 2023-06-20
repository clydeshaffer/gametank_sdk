; ---------------------------------------------------------------------------
; wait.s
; ---------------------------------------------------------------------------
;
; Wait for interrupt and return

.export  _wait, _stop, _nop5, _nop10

; ---------------------------------------------------------------------------
; Wait for interrupt:  Forces the assembler to emit a WAI opcode ($CB)
; ---------------------------------------------------------------------------

.segment  "CODE"

.proc _wait: near

.byte      $CB                    ; Inserts a WAI opcode
           RTS                    ; Return to caller

.endproc

; ---------------------------------------------------------------------------
; Stop:  Forces the assembler to emit a STP opcode ($DB)
; ---------------------------------------------------------------------------

.proc _stop: near

.byte      $DB                    ; Inserts a STP opcode

.endproc

.proc _nop5
        NOP
        NOP
        NOP
        NOP
        NOP
        RTS
.endproc

.proc _nop10
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        RTS
.endproc