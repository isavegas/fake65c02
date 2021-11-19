    .org $8000

test_data:
    byte 0x032
    text "Test data"

print:
    txa
    pha
    tsx
    txa
    sta $0300
    lda #"\n"
    sta $0300
    pla
    rts

reset:
    lda #$ff
    sta $6002

    ldx test_data
    jsr print

    lda #$01
    sta $0301

loop:
    ror
    sta $6000

    jmp loop

    .org $fffc
    .word reset
    .word $0000
