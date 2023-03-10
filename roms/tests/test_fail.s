    org $8000

    include ../lib.s

should_fail: string "This rom should return 1\n"

reset:

    print_str should_fail
    halt 1

    org $fffc
    word reset
    word $0000
