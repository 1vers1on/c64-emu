.segment "RAM"

.segment "RESETV"
    .word main        ; Reset vector

.segment "CODE"
main:
    ; Test immediate addressing
    lda #$55
    ldx #$0f
    ldy #$0a

    ; Test zero-page addressing
    sta $10
    stx $11
    sty $12

    ; Test zero-page,x addressing
    lda $10,x
    sta $20,x

    ; Test zero-page,y addressing
    ldx $10,y
    ldy $12,x

    ; Test absolute addressing
    lda $1234
    sta $1235

    ; Test page crossing
    ldx #$20
    ldy #$20

    lda $01f0,x
    sta $01f0,x
    lda $01f0,y
    sta $01f0,y

    ; Test indexed indirect addressing
    lda ($30,x)
    sta ($30,x)

    ; Test indirect indexed addressing
    lda ($40),y
    sta ($40),y

    jsr test_subroutine

    ; Test branches and jumps
    cmp #$55
    beq branch_target

    lda #<indirect_label
    sta $50
    lda #>indirect_label
    sta $51
    jmp ($50)

indirect_label:
    nop
    jmp test_end

branch_target:
    lda #$AA
loop:
    cmp #$AA
    beq loop
    nop

test_end:
    jmp stack_tests

stack_tests:
    ; Stack operations
    lda #$42
    pha
    php
    pla
    plp

    ; Status flag operations
    clc
    sec
    cli
    sei
    clv

    ; Register transfers
    lda #$37
    tax
    tay
    txa
    tya
    tsx
    txs

    ; Increments and decrements
    inx
    iny
    dex
    dey
    inc $60
    dec $60

    ; Logic operations
    lda #$AA
    and #$0F
    ora #$F0
    eor #$FF

    ; Shifts and rotations
    lda #$81
    asl
    lsr
    rol
    ror
    asl $70
    lsr $70
    
    jmp branch_tests

branch_tests:
    ; Test conditional branches
    lda #$80
    bmi branch_neg
    nop
branch_neg:
    lda #$00
    bpl branch_pos
    nop
branch_pos:
    sec
    bcs branch_carry
    nop
branch_carry:
    clc
    bcc branch_no_carry
    nop
branch_no_carry:
    lda #$00
    beq branch_zero
    nop
branch_zero:
    lda #$01
    bne branch_not_zero
    nop
branch_not_zero:
    
    ; Decimal mode test
    sed
    clc
    lda #$09
    adc #$01
    cld
    
    ; Compare operations
    lda #$40
    cmp #$40
    cpx #$37
    cpy #$37
    
    ; Bit test
    lda #$C0
    bit $80
    
    ; Indirect jumps
    lda #<indirect_target
    sta $90
    lda #>indirect_target
    sta $91
    jmp ($90)
    
indirect_target:
    ; Self-modifying code test
    lda #$EA
    sta modify_instr+1
modify_instr:
    lda #$00

    jmp illegal_op_tests

illegal_op_tests:
    ; Illegal NOPs
    nop
    .byte $80, $FF
    .byte $04, $FF
    .byte $0C, $FF, $FF
    .byte $14, $FF
    .byte $1C, $FF, $FF
    
    ; ALR/ASR
    lda #$FF
    .byte $4B, $55
    
    ; ANC
    lda #$80
    .byte $0B, $C0
    .byte $2B, $80
    
    ; ARR
    lda #$FF
    .byte $6B, $55
    
    ; LAX
    .byte $A7, $20
    .byte $B7, $20
    .byte $AF, $00, $20
    .byte $BF, $00, $20
    .byte $A3, $40
    .byte $B3, $40
    
    ; SAX
    ldx #$0F
    lda #$F0
    .byte $87, $30
    .byte $97, $30
    .byte $8F, $00, $30
    .byte $83, $60
    
    ; DCP
    lda #$42
    .byte $C7, $40
    .byte $D7, $40
    .byte $CF, $00, $40
    .byte $DF, $00, $40
    .byte $DB, $00, $40
    .byte $C3, $60
    .byte $D3, $60
    
    ; ISB/ISC
    lda #$42
    sec
    .byte $E7, $50
    .byte $F7, $50
    .byte $EF, $00, $50
    .byte $FF, $00, $50
    .byte $FB, $00, $50
    .byte $E3, $70
    .byte $F3, $70
    
    ; SLO
    lda #$00
    .byte $07, $60
    .byte $17, $60
    .byte $0F, $00, $60
    .byte $1F, $00, $60
    .byte $1B, $00, $60
    .byte $03, $80
    .byte $13, $80
    
    ; SRE
    lda #$FF
    .byte $47, $70
    .byte $57, $70
    .byte $4F, $00, $70
    .byte $5F, $00, $70
    .byte $5B, $00, $70
    .byte $43, $90
    .byte $53, $90
    
    ; RLA
    lda #$FF
    .byte $27, $80
    .byte $37, $80
    .byte $2F, $00, $80
    .byte $3F, $00, $80
    .byte $3B, $00, $80
    .byte $23, $A0
    .byte $33, $A0
    
    ; RRA
    lda #$42
    clc
    .byte $67, $90
    .byte $77, $90
    .byte $6F, $00, $90
    .byte $7F, $00, $90
    .byte $7B, $00, $90
    .byte $63, $B0
    .byte $73, $B0
    
    ; SBX/AXS
    lda #$F0
    ldx #$0F
    .byte $CB, $05
    
    ; USBC
    lda #$42
    sec
    .byte $EB, $10
    
    ; SHA/AHX
    lda #$FF
    ldx #$0F
    .byte $9F, $00, $40
    .byte $93, $C0
    
    ; SHX/SXA
    ldx #$FF
    .byte $9E, $00, $40
    
    ; SHY/SYA
    ldy #$FF
    .byte $9C, $00, $40
    
    ; TAS/XAS
    lda #$FF
    ldx #$0F
    .byte $9B, $00, $40
    
    ; LAS/LAR
    .byte $BB, $00, $40
    
    ; ANE/XAA
    lda #$FF
    ldx #$0F
    .byte $8B, $42

    jmp edge_case_tests

edge_case_tests:
    ; Zero page wrap-around
    ldx #$FF
    lda #$42
    sta $10
    lda $11,x
    
    ; Page boundary crossing
    ldx #$FF
    ldy #$01
    lda $1000
    lda $0FF1,x
    lda $1000,y
    
    ; Stack tests
    ldx #$FF
    txs
    lda #$01
    pha
    lda #$02
    pha
    lda #$03
    pha
    pla
    pla
    pla
    
    ; Flag tests
    clc
    lda #$FF
    adc #$01
    
    lda #$FF
    eor #$FF
    
    lda #$7F
    adc #$01
    
    clc
    lda #$7F
    adc #$01
    
    ; Decimal mode tests
    sed
    clc
    lda #$09
    adc #$01
    
    clc
    lda #$50
    adc #$50
    
    clc
    lda #$99
    adc #$01
    
    sec
    lda #$00
    sbc #$01
    cld
    
    ; BCD corner cases
    sed
    clc
    lda #$99
    adc #$99
    
    clc
    lda #$09
    sbc #$00
    cld
    
    ; Illegal opcode edge cases
    lda #$55
    ldx #$AA
    .byte $8B, $FF
    
    ; Self-modifying code
    lda #$A9
    sta modify_code
    lda #$FF
    sta modify_code+1
    
modify_code:
    .byte $EA, $EA
    
    ; Mixed mode timing tests
    ldx #$FF
    ldy #$FF
    lda ($01,x)
    lda ($FE),y
    
    ; Interrupt vector setup
    lda #<irq_handler
    sta $FFFE
    lda #>irq_handler
    sta $FFFF
    
    cli
    sei
    
    ; Branch timing pattern
    ldx #10
branch_loop2:
    dex
    bne branch_loop2
    
    ; Read-modify-write tests
    lda #$55
    sta $D0
    inc $D0
    dec $D0
    asl $D0
    lsr $D0
    rol $D0
    ror $D0
    
    ; Overflow flag tests
    clc
    lda #$7F
    adc #$01
    
    clc
    lda #$80
    adc #$FF
    
    ; JMP indirect bug test
    lda #<indirect_target_2
    sta $10FF
    lda #>indirect_target_2
    sta $1100
    lda #$BB
    sta $1000
    jmp ($10FF)
    
indirect_target_2:
    nop
    
    ; BIT instruction tests
    lda #$FF
    ldx #$80
    stx $E0
    bit $E0
    
    ldx #$40
    stx $E1
    bit $E1
    
    ldx #$00
    stx $E2
    bit $E2

    jmp end_loop

irq_handler:
    rti

end_loop:
    jmp end_loop

test_subroutine:
    clc
    adc #$01
    sbc #$02
    rts