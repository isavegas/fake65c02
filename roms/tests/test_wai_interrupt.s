    org $8000

    include ../lib.s
    include strings.s

IRQ_TRIGGERED = $0000

irq_handler:
    lda #$01
    sta IRQ_TRIGGERED
    rti

reset:
    ifdef DEBUG
        print_str m_debug_message
    endif

    print_str m_start

    lda #$00
    sta IRQ_TRIGGERED

    ; Disable interrupts, so execution will continue
    ; immediately
    sei
    print_str m_wai_attempt
    io_out #10
    io_cmd IO_IRQ_REQ
    wai
    lda IRQ_TRIGGERED
    beq wai_error
    print_str m_finish
    halt 0
wai_error:
    print_str m_wai_error
    halt 1

    org $fffc
    word reset
    word irq_handler
