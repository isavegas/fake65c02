    org $8000

    include ../lib.s
    include test_65c02_strings.s

; TODO: Count errors using `err` macro
ERROR_COUNT = $7fff

    macro err,msg
        printstr \msg
    endm

    macro test_ph,r
test_ph\r\():
            save_registers
            lda #$FE
            pha                         ; Put a failure guard onto the stack
            lda #$FE                    ; Failing data
            ld\r\() #$FF                ; Passing data
            ph\r                        ; Attempt to push passing data onto stack
            pla                         ; Pull top of stack to a
            cmp #$FF                    ; Compare a to known passing data
            bne test_ph\r\()_error_     ; Jump to fail state
            printstr m_ph\r\()_success  ; Print pass message
            pla                         ; Pull failure guard from stack
            load_registers
            rts
test_ph\r\()_error_:
            printstr m_ph\r\()_error    ; Print failure message
            load_registers
            rts
    endm

    macro test_pl,r
test_pl\r\():
            save_registers
            lda #$FF                    ; Passing data
            ld\r\() #$FE                ; Failing data
            pha                         ; Push passing data
            pl\r                        ; Attempt to pull passing data from stack to \r
            cp\r\() #$FF                ; Compare \r to known passing data
            bne test_pl\r\()_error_     ; Jump to fail state
            printstr m_pl\r\()_success  ; Print pass message
            load_registers
            rts
test_pl\r\()_error_:
            pla                         ; Pull test data from stack, cleanup
            printstr m_pl\r\()_error    ; Print failure message
            load_registers
            rts
    endm

    test_ph x
    test_pl x

    test_ph y
    test_pl y

test_stz_abs:
    save_registers
    lda #1
    sta $0000
    stz $0000
    lda $0000
    cmp #0
    bne test_stz_abs_error_
    printstr m_stz_abs_success
    load_registers
    rts
test_stz_abs_error_:
    printstr m_stz_abs_error
    load_registers
    rts

test_stz_zp:
    save_registers
    lda #1
    sta $00
    stz $00
    lda $00
    cmp #0
    bne test_stz_zp_error_
    printstr m_stz_zp_success
    load_registers
    rts
test_stz_zp_error_:
    printstr m_stz_abs_error
    load_registers
    rts

test_stz_abs_x:
    save_registers
    lda #1
    sta $0000
    sta $0001
    ldx #0
    stz $0000,x
    lda $0000,x
    cmp #0
    bne test_stz_abs_x_error_
    ldx #1
    stz $0000,x
    lda $0000,x
    cmp #0
    bne test_stz_abs_x_error_
    printstr m_stz_abs_x_success
    load_registers
    rts
test_stz_abs_x_error_:
    printstr m_stz_abs_x_error
    load_registers
    rts

test_stz_zp_x:
    save_registers
    lda #1
    sta $00
    sta $01
    ldx #0
    stz $00,x
    lda $00,x
    cmp #0
    bne test_stz_abs_x_error_
    ldx #1
    stz $00,x
    lda $00,x
    cmp #0
    bne test_stz_zp_x_error_
    printstr m_stz_zp_x_success
    load_registers
    rts
test_stz_zp_x_error_:
    printstr m_stz_zp_x_error
    load_registers
    rts

test_bit_abs_x:
    save_registers
    ldx #%00000000
    stx $0000
    ldx #%00101100
    stx $0001
    ldx #1
    lda #%11000100
    bit %00000000
    bit $0000,x
    beq test_bit_abs_x_error_
    printstr m_bit_abs_x_success
    load_registers
    rts
test_bit_abs_x_error_:
    printstr m_bit_abs_x_error
    load_registers
    rts

test_bit_zp_x:
    save_registers
    ldx #%00000000
    stx $00
    ldx #%00101100
    stx $01
    ldx #1
    lda #%11000100
    bit %00000000
    bit $00,x
    beq test_bit_zp_x_error_
    printstr m_bit_zp_x_success
    load_registers
    rts
test_bit_zp_x_error_:
    printstr m_bit_zp_x_error
    load_registers
    rts

test_jmp_indirect_x:
    save_registers
    lda #<test_jmp_indirect_x_zero_success_
    sta $0000
    lda #>test_jmp_indirect_x_zero_success_
    sta $0001
    lda #<test_jmp_indirect_x_offset_success_
    sta $0002
    lda #>test_jmp_indirect_x_offset_success_
    sta $0003
    ldx #$0000
    jmp ($0000,x)
    jmp test_jmp_indirect_x_error_
test_jmp_indirect_x_zero_success_:
    ldx #2
    jmp ($0000,x)
    jmp test_jmp_indirect_x_error_
test_jmp_indirect_x_offset_success_:
    printstr m_jmp_indirect_x_success
    load_registers
    rts
test_jmp_indirect_x_error_:
    err m_jmp_indirect_x_error
    load_registers
    rts

test_bra_rel:
    save_registers
    bra test_bra_rel_success_
    err m_bra_rel_error
    load_registers
    rts
test_bra_rel_success_:
    printstr m_bra_rel_success
    load_registers
    rts

m_debug_message: string " => DEBUG BUILD\n"
reset:
    ifdef DEBUG
        printstr m_debug_message
    endif
    lda #0
    sta ERROR_COUNT

    printstr m_start

    jsr test_phx
    jsr test_plx

    jsr test_phy
    jsr test_ply

    jsr test_stz_abs
    jsr test_stz_zp
    jsr test_stz_abs_x
    jsr test_stz_zp_x

    jsr test_bit_abs_x
    jsr test_bit_zp_x

    jsr test_jmp_indirect_x

    jsr test_bra_rel

    printstr m_finish
    halt 0

    org $fffc
    word reset
    word $0000
