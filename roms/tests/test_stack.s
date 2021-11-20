    org $8000

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
    rts

m_abc: string "abc\n"
m_def: string "def\n"

reset:
    jsr s_print ; Check if stack works

    ldx #<m_abc ; Lower byte
    lda #>m_abc ; Upper byte
    jsr print   ; Jump to print subroutine

    ldx #<m_abc ; Lower byte
    lda #>m_abc ; Upper byte
    jsr print   ; Jump to print subroutine

    halt 0

    org $fffc
    word reset
    word $0000
