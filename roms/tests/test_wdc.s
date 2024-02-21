    org $8000

    include ../lib.s
    include test_lib.s
    include strings.s

    macro set,value
        pha
        lda \value
        sta $FF
        pla
    endm

test_bbr:
    save_registers
    set #$FE
    bbr0 $FF, test_bbr_success_
    print_err m_bbr_error
    load_registers
    rts
test_bbr_success_:
    print_str m_bbr_success
    load_registers
    rts

test_bbs:
    save_registers
    set #$01
    bbs0 $FF, test_bbs_success_
    print_err m_bbs_error
    load_registers
    rts
test_bbs_success_:
    print_str m_bbs_success
    load_registers
    rts

test_rmb:
    save_registers
    ldx #$01
    stx $FF
    rmb0 $FF
    ldx #$00
    cpx $FF
    beq test_rmb_success_
    print_err m_rmb_error
    load_registers
    rts
test_rmb_success_:
    print_str m_rmb_success
    load_registers
    rts

test_smb:
    save_registers
    ldx #$00
    stx $FF
    smb0 $FF
    ldx #$01
    cpx $FF
    beq test_smb_success_
    print_err m_smb_error
    load_registers
    rts
test_smb_success_:
    print_str m_smb_success
    load_registers
    rts

reset:
    ifdef DEBUG
        print_str m_debug_message
    endif
    init_err

    print_str m_start

    jsr test_bbr
    jsr test_bbs

    jsr test_rmb
    jsr test_smb

    branch_if_error error
    print_str m_success
error:
    halt_with_error_count

    org $fffc
    word reset
    word $0000
