.define IO_IN $7fff
.define IO_CMD $8000
.define IO_OUT $8001
.define SERIAL $8002
.define PRINT_PTR $BA

.define IO_HALT $01
.define IO_BANK_SWICH $fa
.define IO_CHAR_REQ $fb
.define IO_IRQ_REQ $fc
.define IO_HOOK_CALL $fd
.define IO_HOOK_FUNC $fe
.define IO_HOOK $ff

.macro save_registers
    pha
    phx
    phy
.endmacro

.macro load_registers
    ply
    plx
    pla
.endmacro

.macro io_out out
    pha
    lda out
    sta IO_OUT
    pla
.endmacro

.macro io_cmd cmd
    pha
    lda #cmd
    sta IO_CMD
    pla
.endmacro

.macro halt exit_code
    save_registers
    lda #exit_code
    sta IO_OUT
    lda #IO_HALT
    sta IO_CMD
    load_registers
.endmacro

.macro print_char char
    pha
    lda #char
    sta SERIAL
    pla
.endmacro

.macro print_str str
    save_registers
    _push_addr PRINT_PTR
    ldx #<str
    lda #>str
    jsr _print
    _pull_addr PRINT_PTR
    load_registers
.endmacro

.macro print_stri addr
    save_registers
    _push_addr PRINT_PTR
    lda addr + 1
    ldx addr
    jsr _print
    _pull_addr PRINT_PTR
    load_registers
.endmacro

.macro pointer loc,addr
    save_registers
    ldx #<addr
    lda #>addr
    _pointer loc
    load_registers
.endmacro

; Only enabled with -DDEBUG

.ifdef DEBUG
.macro debug n
    pha
    lda #10
    sta IO_OUT
    lda #IO_HOOK
    sta IO_CMD
    pla
.endmacro

.macro debug_func
    pha
    lda #IO_HOOK_FUNC
    sta IO_CMD
    pla
.endmacro

.macro debug_call
    pha
    lda #IO_HOOK_CALL
    sta IO_CMD
    pla
.endmacro

.else

.macro debug n
.endmacro
.macro debug_func
.endmacro
.macro debug_call
.endmacro

.endif


; The following macros clobber any registers they use

.macro _readchar
    io_out #1
    io_cmd IO_CHAR_REQ
    lda IO_IN
.endmacro

.macro _push_addr addr
    lda addr
    pha
    lda addr + 1
    pha
.endmacro

.macro _pull_addr addr
    pla
    sta addr + 1
    pla
    sta addr
.endmacro

.macro _pointer loc
    stx loc
    sta loc + 1
.endmacro