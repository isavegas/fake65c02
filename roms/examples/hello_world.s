    org $8000
    include ../lib.s ; Include some utility macros and subroutines

message: string "Hello world!\n"
reset:
    print_str message
    halt 0

    org $fffc
    word reset
    word $0000
