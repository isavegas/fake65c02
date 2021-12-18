    org $8000

    include ../lib.s
    include strings.s

; TODO: Count errors using `err` macro
ERROR_COUNT = $7fff

    macro err,msg
        print_str \msg
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
            print_str m_ph\r\()_success  ; Print pass message
            pla                         ; Pull failure guard from stack
            load_registers
            rts
test_ph\r\()_error_:
            err m_ph\r\()_error    ; Print failure message
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
            print_str m_pl\r\()_success  ; Print pass message
            load_registers
            rts
test_pl\r\()_error_:
            pla                         ; Pull test data from stack, cleanup
            err m_pl\r\()_error    ; Print failure message
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
    print_str m_stz_abs_success
    load_registers
    rts
test_stz_abs_error_:
    err m_stz_abs_error
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
    print_str m_stz_zp_success
    load_registers
    rts
test_stz_zp_error_:
    err m_stz_abs_error
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
    print_str m_stz_abs_x_success
    load_registers
    rts
test_stz_abs_x_error_:
    err m_stz_abs_x_error
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
    print_str m_stz_zp_x_success
    load_registers
    rts
test_stz_zp_x_error_:
    err m_stz_zp_x_error
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
    print_str m_bit_abs_x_success
    load_registers
    rts
test_bit_abs_x_error_:
    err m_bit_abs_x_error
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
    print_str m_bit_zp_x_success
    load_registers
    rts
test_bit_zp_x_error_:
    err m_bit_zp_x_error
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
    print_str m_jmp_indirect_x_success
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
    print_str m_bra_rel_success
    load_registers
    rts

test_trb_abs:
    save_registers

    lda #$A6
    sta $0000
    lda #$33
    trb $0000
    beq test_trb_abs_error_  ; Zero flag should not be set
    cmp #$33                ; A should not have changed
    bne test_trb_abs_error_  ; Zero flag should be set
    lda $0000
    cmp #$84                ; $00 should be updated
    bne test_trb_abs_error_  ; Zero flag should be set

    lda #$A6
    sta $0001
    lda #$41
    trb $0001
    bne test_trb_abs_error_  ; Zero flag should be set
    cmp #$41                ; A should not have changed
    bne test_trb_abs_error_  ; Zero flag should be set
    lda $0001
    cmp #$A6                ; $00 should be updated
    bne test_trb_abs_error_  ; Zero flag should be set

    print_str m_trb_abs_success
    load_registers
    rts
test_trb_abs_error_:
    err m_trb_abs_error
    load_registers
    rts

test_trb_zp:
    save_registers

    lda #$A6
    sta $00
    lda #$33
    trb $00
    beq test_trb_zp_error_  ; Zero flag should not be set
    cmp #$33                ; A should not have changed
    bne test_trb_zp_error_  ; Zero flag should be set
    lda $00
    cmp #$84                ; $00 should be updated
    bne test_trb_zp_error_  ; Zero flag should be set

    lda #$A6
    sta $01
    lda #$41
    trb $01
    bne test_trb_zp_error_  ; Zero flag should be set
    cmp #$41                ; A should not have changed
    bne test_trb_zp_error_  ; Zero flag should be set
    lda $01
    cmp #$A6                ; $00 should be updated
    bne test_trb_zp_error_  ; Zero flag should be set

    print_str m_trb_zp_success
    load_registers
    rts
test_trb_zp_error_:
    err m_trb_zp_error
    load_registers
    rts

test_tsb_abs:
    save_registers

    lda #$A6
    sta $0000
    lda #$33
    tsb $0000
    beq test_tsb_abs_error_  ; Zero flag should not be set
    cmp #$33                ; A should not have changed
    bne test_tsb_abs_error_  ; Zero flag should be set
    lda $0000
    cmp #$B7                ; $00 should be updated
    bne test_tsb_abs_error_  ; Zero flag should be set

    lda #$A6
    sta $0001
    lda #$41
    tsb $0001
    bne test_tsb_abs_error_  ; Zero flag should be set
    cmp #$41                ; A should not have changed
    bne test_tsb_abs_error_  ; Zero flag should be set
    lda $0001
    cmp #$E7                ; $00 should be updated
    bne test_tsb_abs_error_  ; Zero flag should be set

    print_str m_tsb_abs_success
    load_registers
    rts
test_tsb_abs_error_:
    err m_tsb_abs_error
    load_registers
    rts

test_tsb_zp:
    save_registers

    lda #$A6
    sta $00
    lda #$33
    tsb $00
    beq test_tsb_zp_error_  ; Zero flag should not be set
    cmp #$33                ; A should not have changed
    bne test_tsb_zp_error_  ; Zero flag should be set
    lda $00
    cmp #$B7                ; $00 should be updated
    bne test_tsb_zp_error_  ; Zero flag should be set

    lda #$A6
    sta $01
    lda #$41
    tsb $01
    bne test_tsb_zp_error_  ; Zero flag should be set
    cmp #$41                ; A should not have changed
    bne test_tsb_zp_error_  ; Zero flag should be set
    lda $01
    cmp #$E7                ; $00 should be updated
    bne test_tsb_zp_error_  ; Zero flag should be set

    print_str m_tsb_zp_success
    load_registers
    rts
test_tsb_zp_error_:
    err m_tsb_zp_error
    load_registers
    rts

reset:
    ifdef DEBUG
        print_str m_debug_message
    endif

    lda #0
    sta ERROR_COUNT

    print_str m_start

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

    jsr test_trb_abs
    jsr test_trb_zp

    jsr test_tsb_abs
    jsr test_tsb_zp

    print_str m_finish
    halt 0

    org $fffc
    word reset
    word $0000
