    org $8000

    include ../lib.s
    include test_lib.s
    include strings.s

IRQ_TRIGGERED = $0A

irq_handler:
    inc IRQ_TRIGGERED
    rti

test_wai:
    ifdef DEBUG
        print_str m_debug_message
    endif

    print_str m_start

    lda #0
    sta IRQ_TRIGGERED
    io_out #$FF
    io_cmd IO_IRQ_REQ
    cli
    wai
    sei
    lda IRQ_TRIGGERED
    cmp #0
    beq test_wai_error_
    print_str m_wai_success
    rts
test_wai_error_:
    print_err m_wai_error
    rts

test_wai_nointerrupt:
    sei ; Just in case
    lda #0
    sta IRQ_TRIGGERED
    io_out #$FF
    io_cmd IO_IRQ_REQ
    wai
    lda IRQ_TRIGGERED
    cmp #0
    bne test_wai_nointerrupt_error_
    print_str m_wai_nointerrupt_success
    rts
test_wai_nointerrupt_error_:
    print_err m_wai_nointerrupt_error
    rts

reset:
    sei
    init_err
    jsr test_wai
    jsr test_wai_nointerrupt
    branch_if_error error
    print_str m_success
error:
    halt_with_error_count

    org $fffc
    word reset
    word irq_handler
