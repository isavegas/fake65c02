    org $8000
    include ../lib.s

; TODO

m_todo: string "TODO: Fibonacci benchmark\n"
reset:
    print_str m_todo
    halt 0

    org $fffc
    word reset
    word $0000
