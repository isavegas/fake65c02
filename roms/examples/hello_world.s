    .org $8000

    include ../lib.s

reset:
    lda #"H"
    sta SERIAL
    lda #"e"
    sta SERIAL
    lda #"l"
    sta SERIAL
    lda #"l"
    sta SERIAL
    lda #"o"
    sta SERIAL
    lda #","
    sta SERIAL
    lda #" "
    sta SERIAL
    lda #"w"
    sta SERIAL
    lda #"o"
    sta SERIAL
    lda #"r"
    sta SERIAL
    lda #"l"
    sta SERIAL
    lda #"d"
    sta SERIAL
    lda #"!"
    sta SERIAL
    lda #"\n"
    sta SERIAL

    lda #$00
    sta HALT

    .org $fffc
    .word reset
    .word $0000
