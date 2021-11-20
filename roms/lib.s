IO_IN = $7fff
IO_CMD = $8000
IO_OUT = $8001
SERIAL = $ffff
PRINT_PTR = $BA

IO_HALT = $01
IO_HOOK = $ff

print_char:
    sta SERIAL
    rts

print:
    stx PRINT_PTR     ; Store lower byte of string address
    sta PRINT_PTR + 1 ; Store upper byte of string address
    lda #0            ; Initialize A as 0
    ldy #0            ; Initialize Y as 0
print_:
    lda (PRINT_PTR),y ; Load relative PRINT_PTR + y
    sta SERIAL        ; Store A to serial address
    beq print_done_   ; Jump to done if value loaded into A is zero
    iny               ; Increment Y
    cpy #$ff
    bcs incr_print_ptr_
    jmp print_        ; Loop
incr_print_ptr_
    ldy #0
    inc PRINT_PTR+1
    jmp print_
print_done_:
    rts               ; Return from subroutine

    macro halt,exit_code
        lda #\exit_code
        sta IO_OUT
        lda #IO_HALT
        sta IO_CMD
    endm

    macro printstr,str
        save_registers
        ldx #<\str
        lda #>\str
        jsr print
        load_registers
    endm

    macro save_registers
        pha
        txa
        pha
        tya
        pha
    endm

    macro load_registers
        pla
        tay
        pla
        tax
        pla
    endm

    ifdef DEBUG
    macro debug,n
        pha
        lda #\n
        sta IO_OUT
        lda #IO_HOOK
        sta IO_CMD
        pla
    endm
    else
    macro debug,n
    endm
    endif
