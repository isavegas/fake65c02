    org $8000

    include ../lib.s

RECEIVED_STR = $CC

message: string "echoing input:\n"

readline:
    save_registers
    ldy #0
_readline_loop:
    _readchar
    cmp #3 ; End of text ASCII character
    beq _readline_done
    cmp #0 ; null character
    beq _readline_done
    cmp #10 ; newline
    beq _readline_done
    sta (RECEIVED_STR),y
    iny
    jmp _readline_loop
_readline_done:
    lda #'\0'
    sta (RECEIVED_STR),y
    load_registers
    rts

main:
    pointer RECEIVED_STR, $0FFF
    jsr readline
    pointer RECEIVED_STR, $0CFF
    jsr readline
    print_str message
    print_char '>'
    pointer RECEIVED_STR, $0FFF
    print_stri RECEIVED_STR
    print_char '\n'
    print_char '>'
    pointer RECEIVED_STR, $0CFF
    print_stri RECEIVED_STR
    print_char '\n'
    rts

reset:
    sei
    jsr main
    halt 0

    org $fffc
    word reset
