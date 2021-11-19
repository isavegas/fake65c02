    .org $8000

reset:
    lda #$ff
    sta $6002

    lda #"H"
    sta $0300
    lda #"e"
    sta $0300
    lda #"l"
    sta $0300
    lda #"l"
    sta $0300
    lda #"o"
    sta $0300
    lda #","
    sta $0300
    lda #" "
    sta $0300
    lda #"w"
    sta $0300
    lda #"o"
    sta $0300
    lda #"r"
    sta $0300
    lda #"l"
    sta $0300
    lda #"d"
    sta $0300
    lda #"!"
    sta $0300
    lda #"\n"
    sta $0300
    lda #$01
    sta $0301

loop:
    ror
    sta $6000

    jmp loop

    .org $fffc
    .word reset
    .word $0000
