IO_IN = $7fff
IO_CMD = $8000
IO_OUT = $8001
SERIAL = $8002
PRINT_PTR = $BA
CURSOR_LOCATION = $BC
CHARACTER_MEMORY_LOCATION = $B800
CHARACTER_MEMORY_SIZE = 1920

IO_HALT = $01
IO_BANK_SWICH = $fa
IO_CHAR_REQ = $fb
IO_IRQ_REQ = $fc
IO_HOOK_CALL = $fd
IO_HOOK_FUNC = $fe
IO_HOOK = $ff

string_to_serial:
    stx PRINT_PTR          ; Store lower byte of string address
    sta PRINT_PTR + 1      ; Store upper byte of string address
    ldy #0
.loop:
    lda (PRINT_PTR),y      ; Load relative PRINT_PTR + y
    sta SERIAL             ; Store A to serial address
    beq .done ; Jump to done if value loaded into A is zero
    iny
    bne .loop  ; If y didn't wrap, continue
    inc PRINT_PTR+1        ; If y wrapped, increment high byte of string pointer
    jmp .loop  ; Loop
.done:
    rts

; TODO: Copy string from PRINT_PTR to video memory
; Should write to CHARACTER_MEMORY_LOCATION + CURSOR_LOCATION without
; writing past CHARACTER_MEMORY_LOCATION + CHARACTER_MEMORY_SIZE
; Return value in y register:
;   - 0: Successfully printed string
;   - 1: Reached end of video memory before string terminated
string_to_video:
    stx PRINT_PTR                   ; Store lower byte of string address
    sta PRINT_PTR + 1               ; Store upper byte of string address
    ldy #0
.loop:
    ; Check if the cursor location is within bounds before writing to video memory
    ldx CURSOR_LOCATION+1
    cpx #>CHARACTER_MEMORY_SIZE
    bne .check_high
    ldx CURSOR_LOCATION
    cpx #<CHARACTER_MEMORY_SIZE
    bcs .out_of_bounds
.check_high:
    bcc .write
    bcs .out_of_bounds
.write:
    lda (PRINT_PTR),y
    beq .done
    sta CHARACTER_MEMORY_LOCATION,x ; Store char to video memory
    inc CURSOR_LOCATION
    iny
    bne .loop            ; If y didn't wrap, continue
    inc PRINT_PTR+1                 ; If y wrapped, increment high byte of string pointer
    jmp .loop
.out_of_bounds:
    ldy #1                          ; Ran out of video memory
    rts
.done:
    ldy #0
    rts

    ifdef __NMOS__

    ; compatible with 6502
    macro save_registers
        pha
        txa
        pha
        tya
        pha
    endm

    macro load_registers
        pla
        tay
        pla
        tax
        pla
    endm

    else

    ; 65c02 only
    macro save_registers
        pha
        phx
        phy
    endm

    macro load_registers
        ply
        plx
        pla
    endm

    endif

    macro io_out,out
        pha
        lda \out
        sta IO_OUT
        pla
    endm
    
    macro io_cmd,cmd
        pha
        lda #\cmd
        sta IO_CMD
        pla
    endm

    macro print_char,char
        pha
        lda #\char
        sta SERIAL
        pla
    endm

    macro halt,exit_code
        lda #\exit_code
        sta IO_OUT
        lda #IO_HALT
        sta IO_CMD
    endm

    macro _push_addr,addr
        lda \addr
        pha
        lda \addr + 1
        pha
    endm

    macro _pull_addr,addr
        pla
        sta \addr + 1
        pla
        sta \addr
    endm

    macro pointer,loc,addr
        save_registers
        ldx #<\addr
        lda #>\addr
        _pointer \loc
        load_registers
    endm

    macro _pointer,loc
        stx \loc
        sta \loc + 1
    endm

    ; TODO: Clobbers y (doesn't do it currently)
    macro print_str_video,str
        save_registers
        ldx #<\str
        lda #>\str
        jsr string_to_video
        load_registers
    endm

    macro print_str,str
        save_registers
        _push_addr PRINT_PTR
        ldx #<\str
        lda #>\str
        jsr string_to_serial
        _pull_addr PRINT_PTR
        load_registers
    endm

    macro print_stri,addr
        save_registers
        _push_addr PRINT_PTR
        lda \addr + 1
        ldx \addr
        jsr string_to_serial
        _pull_addr PRINT_PTR
        load_registers
    endm

    macro _readchar
        io_out #1
        io_cmd IO_CHAR_REQ
        lda IO_IN
    endm

    ifdef DEBUG
    macro debug,n
        io_out #\n
        io_cmd IO_HOOK
    endm

    macro debug_func
        io_cmd IO_HOOK_FUNC
    endm

    macro debug_call
        io_cmd IO_HOOK_CALL
    endm

    else

    macro debug,n
    endm
    macro debug_func
    endm
    macro debug_call
    endm

    endif
