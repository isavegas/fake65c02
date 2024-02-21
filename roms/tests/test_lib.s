ERROR_COUNT = $CF

    macro init_err
        pha
        lda #0
        sta ERROR_COUNT
        pla
    endm

    macro print_err,msg
        inc ERROR_COUNT
        print_str \msg
    endm

    macro branch_if_error,location
        lda ERROR_COUNT
        cmp #0
        bne \location
    endm

    macro halt_with_error_count
        lda ERROR_COUNT
        sta IO_OUT
        io_cmd IO_HALT
    endm