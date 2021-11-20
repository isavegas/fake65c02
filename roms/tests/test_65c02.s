    .org $8000

    include ../lib.s

error:
    jsr print
    rts

test_phx:
    lda #%00000010
    ldx #%00000001
    phx
    pla
    and #%00000001
    beq test_phx_error_
    ldx #<m_phx_success
    lda #>m_phx_success
    jsr print
    rts
test_phx_error_:
    pha ; clean up stack
    ldx #<m_phx_error
    lda #>m_phx_error
    jsr print
    rts

test_plx:
    lda #%00000001
    ldx #%00000010
    pha
    plx
    txa
    and #%00000001
    beq test_plx_error_
    ldx #<m_plx_success
    lda #>m_plx_success
    jsr print
    rts
test_plx_error_:
    pla ; clean up stack
    ldx #<m_plx_error
    lda #>m_plx_error
    jsr print
    rts

m_phx_success: .asciiz "PHX success\n"
m_phx_error: .asciiz "PHX error\n"
m_plx_success: .asciiz "PLX success\n"
m_plx_error: .asciiz "PLX error\n"
m_phy_success: .asciiz "PHY success\n"
m_phy_error: .asciiz "PHY error\n"
m_ply_success: .asciiz "PLY success\n"
m_ply_error: .asciiz "PLY error\n"

m_stack_success: .asciiz "Stack success\n"
m_stack_error: .asciiz "Stack error\n"

m_success: .asciiz "Success\n"
m_error: .asciiz "Error\n"
m_start: .asciiz "Starting\n"

reset:
    ldx #<m_start
    lda #>m_start
    jsr print
    jsr test_phx
    jsr test_plx

    lda #$00
    jmp halt

    .org $fffc
    .word reset
    .word $0000
