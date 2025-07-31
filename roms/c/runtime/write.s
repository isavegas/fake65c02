.export _write
.import popax, popptr1, pushax, incsp2, pusha0
.importzp ptr1, ptr2, ptr3, sp

.include "../macros.s"
.include "stdio.inc"

.define buffer ptr1
.define index ptr2
.define count ptr3

; int write (int fd, const void* buf, int count);
.proc _write
    ;debug 1
    ; a and x hold count parameter
    sta count ; store count
    stx count + 1
    ina
    inx
    sta index ; store count + 1
    stx index + 1
    jsr popptr1 ; Buffer address
    jsr popax ; file descriptor

    ; TODO: Add stderr
    ;cmp #1 ; Check that we're writing to stdout
    ;bne not_supported
    ;cpx #0
    ;bne not_supported

begin:
    dec index
    bne outch
    dec index + 1
    beq done

outch:
    ;lda #'.'
    ;sta SERIAL
    ldy #0
    lda (buffer),y
    sta SERIAL

next:
    inc buffer
    bne begin
    inc buffer+1
    jmp begin
    jmp done

not_supported:
    lda #0
    ldx #0
    rts

done:
    lda #1
    ldx #0
    ;jsr pushax
    rts

.endproc
