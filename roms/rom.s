    .org $8000

print:
    lda #"i"
    sta $7001

    lda #"\n"
    sta $7001
    rts

reset:
    lda #"h"
    sta $7001
    jsr print

    lda $01
    sta $7002 ; halt the cpu

    .org $fffc
    .word reset
    .word $0000
