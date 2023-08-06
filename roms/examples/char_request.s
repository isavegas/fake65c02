    org $8000

    include ../lib.s

RECEIVED_STR = $CC00
CHAR_PTR = $FC

loop:
    io_out #1 ; Request synchronously. No wai needed
    io_cmd IO_CHAR_REQ
    ; Emulator pushes character to io_in
    lda IO_IN
    sta (CHAR_PTR),y
    inc CHAR_PTR
    cmp #0
    beq done

    jmp loop
done:
    lda RECEIVED_STR
    sta SERIAL
    print_str RECEIVED_STR
    lda #'\n'
    sta SERIAL
    rts

reset:
    sei
    lda #<RECEIVED_STR
    sta CHAR_PTR
    lda #>RECEIVED_STR
    sta CHAR_PTR + 1
    jsr loop
    halt 0

    org $fffc
    word reset
