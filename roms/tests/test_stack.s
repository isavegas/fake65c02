    .org $8000

    include ../lib.s

s_print_hi:
    lda #"h"
    sta SERIAL
    lda #"i"
    sta SERIAL

    rts

s_print:
    jsr s_print_hi

    lda #"\n"
    sta SERIAL

    lda #$00
    jmp halt

m_abc: .string "abc\n"

reset:
    ldx #<m_abc ; Lower byte
    lda #>m_abc ; Upper byte
    jsr print   ; Jump to print subroutine

    jsr s_print ; Check if stack works

    lda #$01
    sta HALT ; Stack not operational

    .org $fffc
    .word reset
    .word $0000
