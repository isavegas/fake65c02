    org $8000

    include ../lib.s
    include strings.s

; TODO: Count errors using `err` macro
ERROR_COUNT = $7fff

reset:
    ifdef DEBUG
        print_str m_debug_message
    endif

    print_str m_start

    print_str m_stp_attempt
    stp

    print_str m_stp_error
    halt 1

    org $fffc
    word reset
    word $0000
