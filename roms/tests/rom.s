    .org $8000

    include ../lib.s

reset:
    ldx 0x00
    stx HALT

    .org $fffc
    .word reset
    .word $0000
