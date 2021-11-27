    org $8000

    include ../lib.s

print_hi:
    lda #"h"
    sta SERIAL
    lda #"i"
    sta SERIAL
    lda #"\n"
    sta SERIAL

    rts


reset:
    jsr print_hi
    lda #"o"
    sta SERIAL
    lda #"k"
    sta SERIAL
    lda #"\n"
    sta SERIAL
    halt 0

    org $fffc
    word reset
    word $0000
