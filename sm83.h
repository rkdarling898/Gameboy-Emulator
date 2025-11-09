#pragma once

#include "common.h"

#define CARRY_FLAG 4
#define HALF_CARRY_FLAG 5
#define SUBTRACTION_FLAG 6
#define ZERO_FLAG 7

typedef struct {
	uint8_t ime;
	uint8_t rA;
	uint8_t rB;
	uint8_t rC;
	uint8_t rD;
	uint8_t rE;
	uint8_t rF;
	uint8_t rH;
	uint8_t rL;
	uint16_t sp;
	uint16_t pc;
	bool is_halted;
	bool is_running;
} sm83_ctx;

uint8_t read_from_memory (uint8_t *memory, uint16_t addr) {
	// This will be greatly expanded and error checked later
	return *(memory + addr);
}

uint8_t read_next_byte (sm83_ctx *cpu, uint8_t *memory) {
	uint8_t nb = read_from_memory(memory, cpu->pc);

	cpu->pc++;
	return nb;
}

void write_to_memory (uint8_t *memory, uint16_t addr, uint8_t data) {
	// This will also be worked on
	*(memory + addr) = data;
}

void ld_next_byte (sm83_ctx *cpu, uint8_t *memory, uint8_t *dest) {
	*dest = read_next_byte(cpu, memory);
}

void ld_next_u16 (sm83_ctx *cpu, uint8_t *memory, uint8_t *low_b_addr, uint8_t *high_b_addr) {
	*low_b_addr = read_next_byte(cpu, memory);
	*high_b_addr = read_next_byte(cpu, memory);
}

void call_cc (sm83_ctx *cpu, uint8_t *memory, uint8_t flag_index, uint8_t call_if_value) {
	uint8_t h_byte = read_next_byte(cpu, memory);
	uint8_t l_byte = read_next_byte(cpu, memory);

	if (get_bit_u8(&cpu->rF, flag_index) == call_if_value) {
		cpu->sp--;
		write_to_memory(memory, cpu->sp, h_byte);

		cpu->sp--;
		write_to_memory(memory, cpu->sp, l_byte);

		cpu->pc = bytes_to_u16(l_byte, h_byte);
	}
}

void pop_r16 (sm83_ctx *cpu, uint8_t *memory, uint8_t *h_reg, uint8_t *l_reg) {
	*l_reg = read_from_memory(memory, cpu->sp++);
	*h_reg = read_from_memory(memory, cpu->sp++);
}

void push_r16 (sm83_ctx *cpu, uint8_t *memory, uint8_t h_byte, uint8_t l_byte) {
	cpu->sp--;
	write_to_memory(memory, cpu->sp, h_byte);

	cpu->sp--;
	write_to_memory(memory, cpu->sp, l_byte);
}

void push_u16 (sm83_ctx *cpu, uint8_t *memory, uint16_t data) {
	uint8_t l_byte = (uint8_t)((data & 0xFF00) >> 8);
	uint8_t h_byte = (uint8_t)(data & 0x00FF);

	cpu->sp--;
	write_to_memory(memory, cpu->sp, h_byte);

	cpu->sp--;
	write_to_memory(memory, cpu->sp, l_byte);
}

void jp_cc (sm83_ctx *cpu, uint8_t *memory, uint8_t flag_index, uint8_t jump_if_value) {
	uint16_t jp_address = bytes_to_u16(read_next_byte(cpu, memory), read_next_byte(cpu, memory));

	if (get_bit_u8(&cpu->rF, flag_index) == jump_if_value) {
		cpu->pc = jp_address;
	}
}

void jr_cc (sm83_ctx *cpu, uint8_t *memory, uint8_t flag_index, uint8_t jump_if_value) {
	int8_t address_offset = read_next_byte(cpu, memory);

	if (get_bit_u8(&cpu->rF, flag_index) == jump_if_value) {
		if (address_offset < 0) {
			cpu->pc -= (address_offset * -1);
		} else {
			cpu->pc += address_offset;
		}
	}
}

void ret_cc (sm83_ctx *cpu, uint8_t *memory, uint8_t flag_index, uint8_t ret_if_value) {
	if (get_bit_u8(&cpu->rF, flag_index) == ret_if_value) {
		cpu->pc = bytes_to_u16(
			read_from_memory(memory, cpu->sp++),
			read_from_memory(memory, cpu->sp++));
	}
}

uint8_t alu_add (sm83_ctx *cpu, uint8_t a, uint8_t b) {
	uint8_t result = a + b;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);
	set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
	set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, (a & 0x0F) > (result & 0x0F));
	set_bit_u8(&cpu->rF, CARRY_FLAG, a > result);

	return result;
}

uint8_t alu_sub (sm83_ctx *cpu, uint8_t a, uint8_t b) {
	uint8_t result = a - b;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);
	set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, true);
	set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, (a & 0x0F) < (result & 0x0F));
	set_bit_u8(&cpu->rF, CARRY_FLAG, a < result);

	return result;
}

uint8_t alu_and (sm83_ctx *cpu, uint8_t a, uint8_t b) {
	uint8_t result = a & b;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);
	set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
	set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, true);
	set_bit_u8(&cpu->rF, CARRY_FLAG, false);

	return result;
}

uint8_t alu_or (sm83_ctx *cpu, uint8_t a, uint8_t b) {
	uint8_t result = a | b;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);
	set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
	set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, false);
	set_bit_u8(&cpu->rF, CARRY_FLAG, false);

	return result;
}

uint8_t alu_xor (sm83_ctx *cpu, uint8_t a, uint8_t b) {
	uint8_t result = a ^ b;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);
	set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
	set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, false);
	set_bit_u8(&cpu->rF, CARRY_FLAG, false);

	return result;
}

void inc_reg (sm83_ctx *cpu, uint8_t *reg) {
	uint8_t result = *reg + 1;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);
	set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
	set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, (*reg & 0x0F) > (result & 0x0F));

	*reg = result;
}

void dec_reg (sm83_ctx *cpu, uint8_t *reg) {
	uint8_t result = *reg - 1;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);
	set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, true);
	set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, (*reg & 0x0F) < (result & 0x0F));

	*reg = result;
}

void mod_u16_reg (uint8_t *low_reg, uint8_t *high_reg, int8_t value) {
	uint16_t hl_val = bytes_to_u16(*low_reg, *high_reg) + value;

	*low_reg = hl_val & 0x00FF;
	*high_reg = (hl_val & 0xFF00) >> 8;
}

void mod_addr_in_hl (sm83_ctx *cpu, uint8_t *memory, int8_t value) {
	uint8_t addr = bytes_to_u16(cpu->rL, cpu->rH);
	uint8_t addr_val = read_from_memory(memory, addr);
	uint8_t result = addr_val + value;

	set_bit_u8(&cpu->rF, ZERO_FLAG, result == 0);

	if (value < 0) {
		set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, true);
		set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, (addr_val & 0x0F) < (result & 0x0F));
	} else {
		set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
		set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, (addr_val & 0x0F) > (result & 0x0F));
	}

	write_to_memory(memory, addr, result);
}

uint8_t next_instruction (sm83_ctx *cpu, uint8_t *memory) {
	uint8_t op_code = *(memory + cpu->pc);

	cpu->pc++;

	switch (op_code) {
	case 0x00:
		// NOP
		break;
	case 0x01:
		// LD BC, n16
		ld_next_u16(cpu, memory, &cpu->rC, &cpu->rB);
		break;
	case 0x02:
		// LD [BC], A
		write_to_memory(memory, bytes_to_u16(cpu->rC, cpu->rB), cpu->rA);
		break;
	case 0x03:
		// INC BC
		mod_u16_reg(&cpu->rC, &cpu->rB, 1);
		break;
	case 0x04:
		// INC B
		inc_reg(cpu, &cpu->rB);
		break;
	case 0x05:
		// DEC B
		dec_reg(cpu, &cpu->rD);
		break;
	case 0x06:
		// LD B, n8
		ld_next_byte(cpu, memory, &cpu->rB);
		break;
	case 0x07:
		// TODO: refactor op codes using rotate
		// RLCA
		set_bit_u8(&cpu->rF, ZERO_FLAG, false);
		set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
		set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, false);
		set_bit_u8(&cpu->rF, CARRY_FLAG, (get_bit_u8(&cpu->rA, 7) == 1));
		cpu->rA = cpu->rA << 1;
		set_bit_u8(&cpu->rA, 7, (get_bit_u8(&cpu->rF, CARRY_FLAG) == 1));
		break;
	case 0x08:
		// LD [a16], sp
		uint16_t addr = bytes_to_u16(read_next_byte(cpu, memory), read_next_byte(cpu, memory));
		write_to_memory(memory, addr, cpu->sp & 0x00FF);
		write_to_memory(memory, addr + 1, (cpu->sp & 0xFF00) >> 8);
		break;
	case 0x0E:
		// LD C, n8
		ld_next_byte(cpu, memory, &cpu->rC);
		break;
	case 0x11:
		// LD DE, n16
		ld_next_u16(cpu, memory, &cpu->rE, &cpu->rD);
		break;
	case 0x12:
		// LD [DE], A
		write_to_memory(memory, bytes_to_u16(cpu->rE, cpu->rD), cpu->rA);
		break;
	case 0x13:
		// INC DE
		mod_u16_reg(&cpu->rE, &cpu->rD, 1);
		break;
	case 0x14:
		// INC D
		inc_reg(cpu, &cpu->rD);
		break;
	case 0x15:
		// DEC D
		dec_reg(cpu, &cpu->rD);
		break;
	case 0x16:
		// LD D, n8
		ld_next_byte(cpu, memory, &cpu->rD);
		break;
	case 0x17:
		// RLA
		set_bit_u8(&cpu->rF, ZERO_FLAG, false);
		set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
		set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, false);
		set_bit_u8(&cpu->rF, CARRY_FLAG, (get_bit_u8(&cpu->rA, 7) == 1));
		cpu->rA = cpu->rA << 1;
		set_bit_u8(&cpu->rA, 7, (get_bit_u8(&cpu->rF, CARRY_FLAG) == 1));
		break;
	case 0x18:
		// JR e8
		int8_t addr_offset = (int8_t)read_next_byte(cpu, memory);

		if (addr_offset < 0) {
			cpu->pc -= (addr_offset * -1);
		} else {
			cpu->pc += addr_offset;
		}
		break;
	case 0x20:
		// JR NZ, e8
		jr_cc(cpu, memory, ZERO_FLAG, 0);
		break;
	case 0x21:
		// LD HL, n16
		ld_next_u16(cpu, memory, &cpu->rL, &cpu->rH);
		break;
	case 0x22:
		// LD [HL+], A
		write_to_memory(memory, bytes_to_u16(cpu->rC, cpu->rB), cpu->rA);
		mod_u16_reg(&cpu->rL, &cpu->rH, 1);
		break;
	case 0x23:
		// INC HL
		mod_u16_reg(&cpu->rL, &cpu->rH, 1);
		break;
	case 0x24:
		// INC H
		inc_reg(cpu, &cpu->rH);
		break;
	case 0x25:
		// DEC H
		dec_reg(cpu, &cpu->rH);
		break;
	case 0x26:
		// LD H, n8
		ld_next_byte(cpu, memory, &cpu->rH);
		break;
	case 0x27:
		// DAA
		uint8_t adjusted_val = cpu->rA;
		if ((adjusted_val & 0xF) > 9) adjusted_val += 6;
		if (((adjusted_val & 0xF0) >> 4) > 9) adjusted_val += 60;

		cpu->rA = adjusted_val;
		break;
	case 0x28:
		// JR Z, e8
		jr_cc(cpu, memory, ZERO_FLAG, 1);
		break;
	case 0x31:
		// LD SP, n16
		cpu->sp = bytes_to_u16(read_next_byte(cpu, memory), read_next_byte(cpu, memory));
		break;
	case 0x32:
		// LD [HL-], A
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rA);
		mod_u16_reg(&cpu->rL, &cpu->rH, -1);
		break;
	case 0x33:
		// INC SP
		cpu->sp++;
		break;
	case 0x34:
		// INC [HL]
		mod_addr_in_hl(cpu, memory, 1);
		break;
	case 0x35:
		// DEC [HL]
		mod_addr_in_hl(cpu, memory, -1);
		break;
	case 0x36:
		// LD [HL], n8
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), read_next_byte(cpu, memory));
		break;
	case 0x37:
		// SCF
		set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, false);
		set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, false);
		set_bit_u8(&cpu->rF, CARRY_FLAG, true);
		break;
	case 0x38:
		// JR C, e8
		jr_cc(cpu, memory, CARRY_FLAG, 1);
		break;
	case 0x40:
		// LD B, B
		break;
	case 0x41:
		// LD B, C
		cpu->rB = cpu->rC;
		break;
	case 0x42:
		// LD B, D
		cpu->rB = cpu->rD;
		break;
	case 0x43:
		// LD B, E
		cpu->rB = cpu->rE;
		break;
	case 0x44:
		// LD B, H
		cpu->rB = cpu->rH;
		break;
	case 0x45:
		// LD B, L
		cpu->rB = cpu->rL;
		break;
	case 0x46:
		// LD B, [HL]
		cpu->rB = read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH));
	case 0x47:
		// LD B, A
		cpu->rB = cpu->rA;
		break;
	case 0x48:
		// LD C, B
		cpu->rC = cpu->rB;
		break;
	case 0x49:
		// LD C, C
		break;
	case 0x4A:
		// LD C, D
		cpu->rC = cpu->rD;
		break;
	case 0x4B:
		// LD C, E
		cpu->rC = cpu->rE;
		break;
	case 0x4C:
		// LD C, H
		cpu->rC = cpu->rH;
		break;
	case 0x4D:
		// LD C, L
		cpu->rC = cpu->rL;
		break;
	case 0x4E:
		// LD C, [HL]
		cpu->rC = read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH));
		break;
	case 0x4F:
		// LD C, A
		cpu->rC = cpu->rA;
		break;
	case 0x50:
		// LD D, B
		cpu->rD = cpu->rB;
		break;
	case 0x51:
		// LD D, C
		cpu->rD = cpu->rC;
		break;
	case 0x52:
		// LD D, D
		break;
	case 0x53:
		// LD D, E
		cpu->rD = cpu->rE;
		break;
	case 0x54:
		// LD D, H
		cpu->rD = cpu->rH;
		break;
	case 0x55:
		// LD D, L
		cpu->rD = cpu->rL;
		break;
	case 0x56:
		// LD D, [HL]
		cpu->rD = read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH));
		break;
	case 0x57:
		// LD D, A
		cpu->rD = cpu->rA;
		break;
	case 0x58:
		// LD E, B
		cpu->rE = cpu->rB;
		break;
	case 0x59:
		// LD E, C
		cpu->rE = cpu->rC;
		break;
	case 0x5A:
		// LD E, D
		cpu->rE = cpu->rD;
		break;
	case 0x5B:
		// LD E, E
		break;
	case 0x5C:
		// LD E, H
		cpu->rE = cpu->rH;
		break;
	case 0x5D:
		// LD E, L
		cpu->rE = cpu->rL;
		break;
	case 0x5E:
		// LD E, [HL]
		cpu->rE = read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH));
		break;
	case 0x5F:
		// LD E, A
		cpu->rE = cpu->rA;
		break;
	case 0x60:
		// LD H, B
		cpu->rH = cpu->rB;
		break;
	case 0x61:
		// LD H, C
		cpu->rH = cpu->rC;
		break;
	case 0x62:
		// LD H, D
		cpu->rH = cpu->rD;
		break;
	case 0x63:
		// LD H, E
		cpu->rH = cpu->rE;
		break;
	case 0x64:
		// LD H, H
		break;
	case 0x65:
		// LD H, L
		cpu->rH = cpu->rL;
		break;
	case 0x66:
		// LD H, [HL]
		cpu->rH = read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH));
		break;
	case 0x67:
		// LD H, A
		cpu->rH = cpu->rA;
		break;
	case 0x68:
		// LD L, B
		cpu->rL = cpu->rB;
		break;
	case 0x69:
		// LD L, C
		cpu->rL = cpu->rC;
		break;
	case 0x6A:
		// LD L, D
		cpu->rL = cpu->rD;
		break;
	case 0x6B:
		// LD L, E
		cpu->rL = cpu->rE;
		break;
	case 0x6C:
		// LD L, H
		cpu->rL = cpu->rH;
		break;
	case 0x6D:
		// LD L, L
		break;
	case 0x6E:
		// LD L, [HL]
		cpu->rL = read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH));
		break;
	case 0x6F:
		// LD L, A
		cpu->rL = cpu->rA;
		break;
	case 0x70:
		// LD [HL], B
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rB);
		break;
	case 0x71:
		// LD [HL], C
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rC);
		break;
	case 0x72:
		// LD [HL], D
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rD);
		break;
	case 0x73:
		// LD [HL], E
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rE);
		break;
	case 0x74:
		// LD [HL], H
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rH);
		break;
	case 0x75:
		// LD [HL], L
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rL);
		break;
	case 0x76:
		// HALT
		cpu->is_halted = true;
		break;
	case 0x77:
		// LD [HL], A
		write_to_memory(memory, bytes_to_u16(cpu->rL, cpu->rH), cpu->rA);
		break;
	case 0x78:
		// LD A, B
		cpu->rA = cpu->rB;
		break;
	case 0x79:
		// LD A, C
		cpu->rA = cpu->rC;
		break;
	case 0x7A:
		// LD A, D
		cpu->rA = cpu->rD;
		break;
	case 0x7B:
		// LD A, E
		cpu->rA = cpu->rE;
		break;
	case 0x7C:
		// LD A, H
		cpu->rA = cpu->rH;
		break;
	case 0x7D:
		// LD A, L
		cpu->rA = cpu->rL;
		break;
	case 0x7E:
		// LD A, [HL]
		cpu->rA = read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH));
		break;
	case 0x7F:
		// LD A, A
		break;
	case 0x80:
		// ADD A, B
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rB);
		break;
	case 0x81:
		// ADD A, C
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rC);
		break;
	case 0x82:
		// ADD A, D
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rD);
		break;
	case 0x83:
		// ADD A, E
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rE);
		break;
	case 0x84:
		// ADD A, H
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rH);
		break;
	case 0x85:
		// ADD A, L
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rL);
		break;
	case 0x86:
		// ADD A, [HL]
		cpu->rA = alu_add(cpu, cpu->rA, read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH)));
		break;
	case 0x87:
		// ADD A, A
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rA);
		break;
	case 0x88:
		// ADC A, B
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rB + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x89:
		// ADC A, C
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rC + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x8A:
		// ADC A, D
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rD + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x8B:
		// ADC A, E
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rE + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x8C:
		// ADC A, H
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rH + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x8D:
		// ADC A, L
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rL + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x8E:
		// ADC A, [HL]
		cpu->rA = alu_add(cpu, cpu->rA,
			read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH)) +
			get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x8F:
		// ADC A, A
		cpu->rA = alu_add(cpu, cpu->rA, cpu->rA + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x90:
		// SUB A, B
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rB);
		break;
	case 0x91:
		// SUB A, C
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rC);
		break;
	case 0x92:
		// SUB A, D
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rD);
		break;
	case 0x93:
		// SUB A, E
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rE);
		break;
	case 0x94:
		// SUB A, H
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rH);
		break;
	case 0x95:
		// SUB A, L
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rL);
		break;
	case 0x96:
		// SUB A, [HL]
		cpu->rA = alu_sub(cpu, cpu->rA,
			read_from_memory(memory, read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH))));
		break;
	case 0x97:
		// SUB A, A
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rA);
		break;
	case 0x98:
		// SBC A, B (a - b - c = a - (b + c); Thank you distributive property)
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rB + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x99:
		// SBC A, C
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rC + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x9A:
		// SBC A, D
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rD + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x9B:
		// SBC A, E
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rE + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x9C:
		// SBC A, H
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rH + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x9D:
		// SBC A, L
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rL + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x9E:
		// SBC A, [HL]
		cpu->rA = alu_sub(cpu, cpu->rA,
			read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH)) +
			get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0x9F:
		// SBC A, A
		cpu->rA = alu_sub(cpu, cpu->rA, cpu->rA + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0xA0:
		// AND A, B
		cpu->rA = alu_and(cpu, cpu->rA, cpu->rB);
		break;
	case 0xA1:
		// AND A, C
		cpu->rA = alu_and(cpu, cpu->rA, cpu->rC);
		break;
	case 0xA2:
		// AND A, D
		cpu->rA = alu_and(cpu, cpu->rA, cpu->rD);
		break;
	case 0xA3:
		// AND A, E
		cpu->rA = alu_and(cpu, cpu->rA, cpu->rE);
		break;
	case 0xA4:
		// AND A, H
		cpu->rA = alu_and(cpu, cpu->rA, cpu->rH);
		break;
	case 0xA5:
		// AND A, L
		cpu->rA = alu_and(cpu, cpu->rA, cpu->rL);
		break;
	case 0xA6:
		// AND A, [HL]
		cpu->rA = alu_and(cpu, cpu->rA, read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH)));
		break;
	case 0xA7:
		// AND A, A
		cpu->rA = alu_and(cpu, cpu->rA, cpu->rB);
		break;
	case 0xA8:
		// XOR A, B
		cpu->rA = alu_xor(cpu, cpu->rA, cpu->rB);
		break;
	case 0xA9:
		// XOR A, C
		cpu->rA = alu_xor(cpu, cpu->rA, cpu->rC);
		break;
	case 0xAA:
		// XOR A, D
		cpu->rA = alu_xor(cpu, cpu->rA, cpu->rD);
		break;
	case 0xAB:
		// XOR A, E
		cpu->rA = alu_xor(cpu, cpu->rA, cpu->rE);
		break;
	case 0xAC:
		// XOR A, H
		cpu->rA = alu_xor(cpu, cpu->rA, cpu->rH);
		break;
	case 0xAD:
		// XOR A, L
		cpu->rA = alu_xor(cpu, cpu->rA, cpu->rL);
		break;
	case 0xAE:
		// XOR A, [HL]
		cpu->rA = alu_xor(cpu, cpu->rA, read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH)));
		break;
	case 0xAF:
		// XOR A, A (Clear accumulator)
		cpu->rA = alu_xor(cpu, cpu->rA, cpu->rA);
		break;
	case 0xB0:
		// OR A, B
		cpu->rA = alu_or(cpu, cpu->rA, cpu->rB);
		break;
	case 0xB1:
		// OR A, C
		cpu->rA = alu_or(cpu, cpu->rA, cpu->rC);
		break;
	case 0xB2:
		// OR A, D
		cpu->rA = alu_or(cpu, cpu->rA, cpu->rD);
		break;
	case 0xB3:
		// OR A, E
		cpu->rA = alu_or(cpu, cpu->rA, cpu->rE);
		break;
	case 0xB4:
		// OR A, H
		cpu->rA = alu_or(cpu, cpu->rA, cpu->rH);
		break;
	case 0xB5:
		// OR A, L
		cpu->rA = alu_or(cpu, cpu->rA, cpu->rL);
		break;
	case 0xB6:
		// OR A, [HL]
		cpu->rA = alu_or(cpu, cpu->rA, read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH)));
		break;
	case 0xB7:
		// OR A, A
		cpu->rA = alu_or(cpu, cpu->rA, cpu->rA);
		break;
	case 0xB8:
		// CP A, B
		alu_sub(cpu, cpu->rA, cpu->rB);
		break;
	case 0xB9:
		// CP A, C
		alu_sub(cpu, cpu->rA, cpu->rC);
		break;
	case 0xBA:
		// CP A, D
		alu_sub(cpu, cpu->rA, cpu->rD);
		break;
	case 0xBB:
		// CP A, E
		alu_sub(cpu, cpu->rA, cpu->rE);
		break;
	case 0xBC:
		// CP A, H
		alu_sub(cpu, cpu->rA, cpu->rH);
		break;
	case 0xBD:
		// CP A, L
		alu_sub(cpu, cpu->rA, cpu->rL);
		break;
	case 0xBE:
		// CP A, [HL]
		alu_sub(cpu, cpu->rA, read_from_memory(memory, bytes_to_u16(cpu->rL, cpu->rH)));
		break;
	case 0xBF:
		// CP A, A
		alu_sub(cpu, cpu->rA, cpu->rA);
		break;
	case 0xC0:
		// RET NZ
		ret_cc(cpu, memory, ZERO_FLAG, 0);
		break;
	case 0xC1:
		// POP BC
		pop_r16(cpu, memory, &cpu->rB, &cpu->rC);
		break;
	case 0xC2:
		// JP NZ, a16
		jp_cc(cpu, memory, ZERO_FLAG, 0);
		break;
	case 0xC3:
		// JP a16
		cpu->pc = bytes_to_u16(read_next_byte(cpu, memory), read_next_byte(cpu, memory));
		break;
	case 0xC4:
		// CALL NZ, a16
		call_cc(cpu, memory, ZERO_FLAG, 0);
		break;
	case 0xC5:
		// PUSH BC
		push_r16(cpu, memory, cpu->rB, cpu->rC);
		break;
	case 0xC6:
		// ADD A, n8
		cpu->rA = alu_add(cpu, cpu->rA, read_next_byte(cpu, memory));
		break;
	case 0xC7:
		// RST $00
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x00;
		break;
	case 0xC8:
		// RET Z
		ret_cc(cpu, memory, ZERO_FLAG, 1);
		break;
	case 0xC9:
		// RET
		cpu->pc = bytes_to_u16(
			read_from_memory(memory, cpu->sp++),
			read_from_memory(memory, cpu->sp++));
		break;
	case 0xCA:
		// JP Z, a16
		jp_cc(cpu, memory, ZERO_FLAG, 1);
		break;
	case 0xCB:
		// PREFIX (this is going to be another table of joy to work out later)
		break;
	case 0xCC:
		// CALL Z, a16
		call_cc(cpu, memory, ZERO_FLAG, 1);
		break;
	case 0xCD:
		// CALL a16
		uint8_t l_byte = read_next_byte(cpu, memory);
		uint8_t h_byte = read_next_byte(cpu, memory);

		cpu->sp--;
		write_to_memory(memory, cpu->sp, l_byte);

		cpu->sp--;
		write_to_memory(memory, cpu->sp, h_byte);

		cpu->pc = bytes_to_u16(l_byte, h_byte);
		break;
	case 0xCE:
		// ADC A, n8
		cpu->rA = alu_add(cpu, cpu->rA,
			read_next_byte(cpu, memory) + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0xCF:
		// RST $08
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x08;
		break;
	case 0xD0:
		// RET NC
		ret_cc(cpu, memory, CARRY_FLAG, 0);
		break;
	case 0xD1:
		// POP DE
		pop_r16(cpu, memory, &cpu->rD, &cpu->rE);
		break;
	case 0xD2:
		// JP NC, a16
		jp_cc(cpu, memory, CARRY_FLAG, 0);
		break;
	case 0xD4:
		// CALL NC, a16
		call_cc(cpu, memory, CARRY_FLAG, 0);
		break;
	case 0xD5:
		// PUSH DE
		push_r16(cpu, memory, cpu->rD, cpu->rE);
		break;
	case 0xD6:
		// SUB A, n8
		cpu->rA = alu_sub(cpu, cpu->rA, read_next_byte(cpu, memory));
		break;
	case 0xD7:
		// RST $10
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x10;
		break;
	case 0xD8:
		// RET C
		ret_cc(cpu, memory, CARRY_FLAG, 1);
		break;
	case 0xD9:
		// RETI
		cpu->pc = bytes_to_u16(
			read_from_memory(memory, cpu->sp++),
			read_from_memory(memory, cpu->sp++));
		
		cpu->ime = 1;
		break;
	case 0xDA:
		// JP C, a16
		jp_cc(cpu, memory, CARRY_FLAG, 1);
		break;
	case 0xDC:
		// CALL C, a16
		call_cc(cpu, memory, CARRY_FLAG, 1);
		break;
	case 0xDE:
		// SBC A, n8
		cpu->rA = alu_sub(cpu, 
			cpu->rA, read_next_byte(cpu, memory) + get_bit_u8(&cpu->rF, CARRY_FLAG));
		break;
	case 0xDF:
		// RST $18
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x18;
		break;
	case 0xE0:
		// LDH [a8], A
		write_to_memory(memory, bytes_to_u16(read_next_byte(cpu, memory), 0xFF), cpu->rA);
		break;
	case 0xE1:
		// POP HL
		pop_r16(cpu, memory, &cpu->rH, &cpu->rL);
		break;
	case 0xE2:
		// LDH [C], A
		write_to_memory(memory, bytes_to_u16(cpu->rC, 0xFF), cpu->rA);
		break;
	case 0xE5:
		// PUSH HL
		push_r16(cpu, memory, cpu->rH, cpu->rL);
		break;
	case 0xE6:
		// AND A, n8
		cpu->rA = alu_add(cpu, cpu->rA, read_next_byte(cpu, memory));
		break;
	case 0xE7:
		// RST $20
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x20;
		break;
	// Can't wait to get to the chaos that is 0xE8
	case 0xE9:
		// JP HL
		cpu->pc = bytes_to_u16(cpu->rL, cpu->rH);
		break;
	case 0xEA:
		// LD [a16], A
		write_to_memory(memory,
			bytes_to_u16(read_next_byte(cpu, memory), read_next_byte(cpu, memory)), cpu->rA);
		break;
	case 0xEE:
		// XOR A, n8
		cpu->rA = alu_xor(cpu, cpu->rA, read_next_byte(cpu, memory));
		break;
	case 0xEF:
		// RST $28
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x28;
		break;
	case 0xF0:
		// LDH A, [a8]
		cpu->rA = read_from_memory(memory, bytes_to_u16(read_next_byte(cpu, memory), 0xFF));
		break;
	case 0xF1:
		// POP AF
		pop_r16(cpu, memory, &cpu->rA, &cpu->rF);
		break;
	case 0xF2:
		// LDH A, [C]
		cpu->rA = read_from_memory(memory, bytes_to_u16(cpu->rC, 0xFF));
		break;
	case 0xF3:
		// DI
		cpu->ime = 0;
		break;
	case 0xF5:
		// PUSH AF
		push_r16(cpu, memory, cpu->rA, cpu->rF);
		break;
	case 0xF6:
		// OR A, n8
		cpu->rA = alu_or(cpu, cpu->rA, read_next_byte(cpu, memory));
		break;
	case 0xF7:
		// RST $30
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x30;
		break;
	case 0xF9:
		// LD SP, HL
		cpu->sp = bytes_to_u16(cpu->rL, cpu->rH);
		break;
	case 0xFA:
		// LD A, [a16]
		cpu->rA = read_from_memory(memory,
			bytes_to_u16(read_next_byte(cpu, memory), read_next_byte(cpu, memory)));
		break;
	case 0xFB:
		// EI
		cpu->ime = 1;
		break;
	case 0xFE:
		// CP A, n8
		alu_sub(cpu, cpu->rA, read_next_byte(cpu, memory));
		break;
	case 0xFF:
		// RST $38
		push_u16(cpu, memory, cpu->pc);
		cpu->pc = 0x38;
		break;
	default:
		cpu->is_running = false;
		printf("Was unable to process instruction 0x%02X\n", op_code);
		break;
	}

	return op_code;
}