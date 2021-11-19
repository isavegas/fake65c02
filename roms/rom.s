    .org $8000

print:
    lda #"\n"
    sta $0300
    rts

reset:
    lda #$ff
    sta $6002

    lda #"h"
    sta $0300
    jsr print

    sta $0301 ; halt the cpu

    .org $fffc
    .word reset
    .word $0000
