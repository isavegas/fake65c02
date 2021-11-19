    .org $8000

reset:
    lda #$ff
    sta $6002

    lda #"H"
    sta $7001
    lda #"e"
    sta $7001
    lda #"l"
    sta $7001
    lda #"l"
    sta $7001
    lda #"o"
    sta $7001
    lda #","
    sta $7001
    lda #" "
    sta $7001
    lda #"w"
    sta $7001
    lda #"o"
    sta $7001
    lda #"r"
    sta $7001
    lda #"l"
    sta $7001
    lda #"d"
    sta $7001
    lda #"!"
    sta $7001
    lda #"\n"
    sta $7001
    lda #$00
    sta $7002

    .org $fffc
    .word reset
    .word $0000
