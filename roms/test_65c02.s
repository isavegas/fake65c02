    .org $8000

print:
    lda #"\n"
    sta $0300

    lda $01
    sta $0301 ; halt the cpu

    rts

reset:
    lda #$ff
    sta $6002

    lda #"h"
    sta $0300
    jsr print

    lda $00
    sta $0301 ; halt the cpu

    .org $fffc
    .word reset
    .word $0000
