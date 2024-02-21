.feature string_escapes

.import _print, read_char, write_char

.export _main

.include "macros.s"

RECEIVED_STR = $CC

hello_message: .asciiz ">> Hello from 65c02 code compiled by ca65\n"
error_message: .asciiz "Error!\n"

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
    lda #0
    sta (RECEIVED_STR),y
    load_registers
    rts

writeline:
    print_stri RECEIVED_STR
    print_char 10
    rts

_main:
    sei
    debug 1
    pointer RECEIVED_STR, $0FFF
    jsr readline
    ldy #0
    lda (RECEIVED_STR),y
    cmp #0
    beq err
    pointer RECEIVED_STR, $0EFF
    jsr readline
    ldy #0
    lda (RECEIVED_STR),y
    cmp #0
    beq err
    print_str hello_message
    pointer RECEIVED_STR, $0FFF
    jsr writeline
    pointer RECEIVED_STR, $0EFF
    jsr writeline
    debug 0
    stp
err:
    print_str error_message
    rts
