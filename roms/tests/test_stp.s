    org $8000

    include ../lib.s
    include strings.s

stp_message: string "\nThis test succeeded if it stopped running after this message is printed\n"

reset:
    ifdef DEBUG
        print_str m_debug_message
    endif

    print_str m_stp_attempt
    print_str stp_message
    stp
    print_str m_stp_error
    halt 1

    org $fffc
    word reset
    word $0000
