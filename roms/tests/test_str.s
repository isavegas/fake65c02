    org $8000

    include ../lib.s
    include test_lib.s
    include strings.s

; Make sure that using print_str doesn't clobber wherever it stores a pointer to the string
print_str_noclobber:
    lda #12
    sta PRINT_PTR
    lda #34
    sta PRINT_PTR + 1
    print_str m_start
    lda PRINT_PTR
    cmp #12
    bne print_str_noclobber_error
    lda PRINT_PTR + 1
    cmp #34
    bne print_str_noclobber_error
    print_str m_print_str_noclobber_success
    rts
print_str_noclobber_error:
    print_err m_print_str_noclobber_fail
    rts

reset:
    ifdef DEBUG
        print_str m_debug_message
    endif
    init_err
    jsr print_str_noclobber
    branch_if_error error
    print_str m_success
    halt 0
error:
    print_str m_error
    halt_with_error_count

    org $fffc
    word reset
    word $0000
