.globl tag_memset


tag_memset:
	move a4, a0
	li a7, 8
	and a6, a2, ~7
	blt a2, a7, .Ltiny
	li a7,0
.Lrolling:
	stag a1, 0(a4)
	stag a1, 0x10(a4)
	stag a1, 0x20(a4)
	stag a1, 0x30(a4)
	stag a1, 0x40(a4)
	stag a1, 0x50(a4)
	stag a1, 0x60(a4)
	stag a1, 0x70(a4)
	addi a7, a7, 8
	addi a4, a4, 0x80
	blt a7, a6, .Lrolling
	beq a7, a2, .Lend
.Ltiny:
	sub a3, a2, a6
	li a7, 7
	sub a3, a7, a3
	sll a3, a3, 2
1:	auipc t0, 0
	add a3, a3, t0
	addi a3, a3, 0x10
.option push
.option norvc
	jr a3//, %pcrel_lo(1b)
.Ltable:
	stag a1, 0x60(a4)
	stag a1, 0x50(a4)
	stag a1, 0x40(a4)
	stag a1, 0x30(a4)
	stag a1, 0x20(a4)
	stag a1, 0x10(a4)
	stag a1, (a4)
.option pop
.Lend:
	ret
