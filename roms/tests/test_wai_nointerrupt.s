    org $8000

    include ../lib.s
    include strings.s

irq_handler:
    print_str m_wai_error
    halt 1
    rti

reset:
    ifdef DEBUG
        print_str m_debug_message
    endif

    print_str m_start


    ; Disable interrupts, so execution will continue
    ; immediately after wai. No way to detect if it
    ; waited any amount of clock cycles.
    sei
    print_str m_wai_attempt
    io_out #10
    io_cmd IO_IRQ_REQ
    wai
    print_str m_wai_success

    print_str m_finish
    halt 0

    org $fffc
    word reset
    word irq_handler
