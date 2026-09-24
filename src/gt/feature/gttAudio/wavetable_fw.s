; ---------------------------------------------------------------------------
; wavetable_fw.s
; ---------------------------------------------------------------------------
;
; Embeds Burdock's wavetable ACP firmware from the Rust SDK:
; https://github.com/dwbrite/gametank-sdk/blob/61755e530b12172f45243345dd515fd37f819feb/rom-template/gametank/audiofw/wavetable.bin

.include "../../../gen/modules_enabled.inc"

.ifdef ENABLE_MODULE_GTTAUDIO
.export _WavetableFWPkg

.segment "COMMON"
_WavetableFWPkg:
    .incbin "build/assets/wavetable.bin.deflate"
.endif
