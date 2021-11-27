    org $8000
    include ../lib.s

test_function:
    nop
    nop
    rts

test_function2:
    debug_func ; This function should have debug output for every invocation
    nop
    nop
    rts

m_test_function: string "test_function :: "
m_test_function2: string "test_function2 :: debug on\n" ; Debug always on for this function
m_debug_off: string "debug off\n"
m_debug_on: string "debug on\n"

reset:
    ; Call test_function with debug to check debug_call
    print_str m_test_function
    print_str m_debug_on
    debug_call
    jsr test_function

    ; Call test_function2, which is defined with debug_func
    print_str m_test_function2
    jsr test_function2
    print_str m_test_function2
    jsr test_function2

    ; Call test_function without debug output, to check if debug_func correctly ends
    print_str m_test_function
    print_str m_debug_off
    jsr test_function

    halt 0

    org $fffc
    word reset
    word $0000
