    .org $8000

    include ../lib.s

reset:
    ldx #$00
    jsr halt

    .org $fffc
    .word reset
    .word $0000
