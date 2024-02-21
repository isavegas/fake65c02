    org $8000
    include ../lib.s

test_function_nodebug:
    nop
    nop
    rts

test_function_debug:
    debug_func ; This function should have debug output for every invocation
    nop
    nop
    rts

m_test_function_nodebug: string "test_function_nodebug :: "
m_test_function_debug: string "test_function_debug :: debug on\n" ; Debug always on for this function
m_debug_off: string "debug off\n"
m_debug_on: string "debug on\n"

reset:
    ; Call test_function with debug to check debug_call
    print_str m_test_function_nodebug
    print_str m_debug_on
    debug_call
    jsr test_function_nodebug
    print_str m_debug_off

    ; Call test_function2, which is defined with debug_func
    print_str m_test_function_debug
    jsr test_function_debug
    print_str m_test_function_debug
    jsr test_function_debug

    ; Call test_function without debug output, to check if debug_func correctly ends
    print_str m_test_function_nodebug
    print_str m_debug_off
    jsr test_function_nodebug

    halt 0

    org $fffc
    word reset
    word $0000
