.export _init, _exit, _print, read_char, write_char
.import _main

.include "macros.s"

.export __STARTUP__ : absolute = 1
.import __RAM_START__, __RAM_SIZE__

.PC02

.zeropage
.include "zeropage.inc"
sp: .res 2
sreg: .res 2

.segment "STARTUP"

; TODO: implement asynchronous request. Continue CPU execution and use IRQ to provide char.
read_char:
    ;lda #1 ; Request synchronously. No wai needed
    ;sta IO_OUT
    ;lda #IO_CHAR_REQ
    ;sta IO_CMD
    io_out #1
    io_cmd IO_CHAR_REQ
    lda IO_IN
    rts

write_char:
    sta SERIAL
    rts

; Takes x and a as lower and upper bytes of address to null-terminated string
_print:
    phy               ; Prevent clobbering y
    stx PRINT_PTR     ; Store lower byte of string address
    sta PRINT_PTR + 1 ; Store upper byte of string address
    lda #0            ; Initialize A as 0
    ldy #0            ; Initialize Y as 0
_print_loop:
    lda (PRINT_PTR),y ; Load relative PRINT_PTR + y
    sta SERIAL        ; Store A to serial address
    cmp 0
    beq _print_done   ; Jump to done if value loaded into A is zero
    iny
    cpy #$ff
    bcs _incr_print_ptr
    jmp _print_loop
_incr_print_ptr:
    ldy #0
    inc PRINT_PTR+1
    jmp _print
_print_done:
    ply
    rts

_init:
    ldx #$FF
    txs
    cld
    lda #<(__RAM_START__ + __RAM_SIZE__)
    sta sp
    lda #>(__RAM_START__ + __RAM_SIZE__)
    sta sp+1
    jsr _main
    cmp #0
    beq _exit
    sta IO_OUT
    lda #IO_HALT
    sta IO_CMD

_exit:
    halt 0

.segment "VECTORS"
    .word 0
    .word _init
    .word 0
