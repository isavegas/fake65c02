IO_IN = $7fff
IO_OUT = $8001
HALT = $8001
SERIAL = $ffff
PRINT_PTR = $BA

;init_mem:
;    pha         ; a -> stack
;    txa         ; x -> a
;    pha         ; a -> stack
;    tya         ; y -> a
;    pha         ; a -> stack
;    ldy #$00
;    ldx #$00
;set_io_:
;    stx IO_IN,y ; x -> $BC+x
;    iny
;    tya
;    cmp #$03
;    beq set_io_done_
;    jmp set_io_
;set_io_done_:
;    pla         ; stack -> a
;    tay         ; a -> y
;    pla         ; stack -> a
;    tax         ; a -> x
;    pla         ; stack -> a
;    rts

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

halt:
    sta HALT
