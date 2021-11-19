    .org $8000

    include ../lib.s

print_hi:
    ldx #"h"
    stx SERIAL
    ldx #"i"
    stx SERIAL

    rts

print:
    jsr print_hi

    ldx #"\n"
    stx SERIAL

    ldx #$00
    jsr halt


reset:
    jsr print ; Check if stack works

    ldx #$01
    ldx HALT ; Stack not operational

    .org $fffc
    .word reset
    .word $0000
