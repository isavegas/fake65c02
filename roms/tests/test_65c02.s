    org $8000

    include ../lib.s

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

m_debug: string "DEBUG\n"
m_phx_success: string "PHX pass\n"
m_phx_error: string "PHX fail\n"
m_plx_success: string "PLX pass\n"
m_plx_error: string "PLX fail\n"
m_phy_success: string "PHY pass\n"
m_phy_error: string "PHY fail\n"
m_ply_success: string "PLY pass\n"
m_ply_error: string "PLY fail\n"
m_stz_abs_success: string "STZ abs pass\n"
m_stz_abs_error: string "STZ abs fail\n"
m_stz_zp_success: string "STZ zp pass\n"
m_stz_zp_error: string "STZ zp fail\n"
m_stz_abs_x_success: string "STZ abs,x pass\n"
m_stz_abs_x_error: string "STZ abs,x fail\n"
m_stz_zp_x_success: string "STZ zp,x pass\n"
m_stz_zp_x_error: string "STZ zp,x fail\n"
m_bit_abs_x_success: string "BIT abs,x pass\n"
m_bit_abs_x_error: string "BIT abs,x fail\n"
m_bit_zp_x_success: string "BIT zp,x pass\n"
m_bit_zp_x_error: string "BIT zp,x fail\n"

m_stack_success: string "Stack pass\n"
m_stack_error: string "Stack fail\n"

m_success: string "Success\n"
m_error: string "Error\n"
m_finish: string "Finished\n"
m_start: string "Starting\n"

reset:
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

    printstr m_finish

    lda #$00
    jmp halt

    org $fffc
    word reset
    word $0000
