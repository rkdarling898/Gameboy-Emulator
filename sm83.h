#pragma once

#include "common.h"

#define CARRY_FLAG 4
#define HALF_CARRY_FLAG 5
#define SUBTRACTION_FLAG 6
#define ZERO_FLAG 7

typedef struct {
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

uint16_t read_next_u16 (sm83_ctx *cpu, uint8_t *memory) {
	return bytes_to_u16(
		read_next_byte(cpu, memory),
		read_next_byte(cpu, memory)
	);
}

void write_to_bus (uint8_t *memory, uint16_t addr, uint8_t data) {
	// This will also be worked on
	*(memory + addr) = data;
}

void ld_next_byte (sm83_ctx *cpu, uint8_t *memory, uint8_t *dest) {
	*dest = read_next_byte(cpu, memory);
}

void push_to_stack (sm83_ctx *cpu, uint8_t *memory, uint16_t data) {
	uint8_t low_byte = (uint8_t)((data & 0xFF00) >> 8);
	uint8_t high_byte = (uint8_t)(data & 0x00FF);

	cpu->sp--;
	write_to_bus(memory, cpu->sp, high_byte);

	cpu->sp--;
	write_to_bus(memory, cpu->sp, low_byte);
}

void jr_conditional (sm83_ctx *cpu, uint8_t *memory, uint8_t flag_index, uint8_t jump_if_value) {
	int8_t address_offset = read_next_byte(cpu, memory);

	if (get_bit_u8(&cpu->rF, flag_index) == jump_if_value) {
		cpu->pc += address_offset;
	}
}

uint8_t next_instruction (sm83_ctx *cpu, uint8_t *memory) {
	uint8_t op_code = *(memory + cpu->pc);

	cpu->pc++;

	switch (op_code) {
	case 0x00:
		// NOP
		break;
	case 0x05:
		// DEC B
		set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, true);

		if (cpu->rB == 1) {
			set_bit_u8(&cpu->rF, ZERO_FLAG, true);
			set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, false);
		} else if (cpu->rB == 0) {
			set_bit_u8(&cpu->rF, ZERO_FLAG, false);
			set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, true);
		} else {
			set_bit_u8(&cpu->rF, ZERO_FLAG, false);
			set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, false);
		}

		cpu->rB--;
		break;
	case 0x06:
		// LD B, n8
		ld_next_byte(cpu, memory, &cpu->rB);
		break;
	case 0x0E:
		// LD C, n8
		ld_next_byte(cpu, memory, &cpu->rC);
		break;
	case 0x20:
		// JR NZ, e8
		jr_conditional(cpu, memory, ZERO_FLAG, 0);
		break;
	case 0x21:
		// LD HL, n16
		cpu->rL = read_next_byte(cpu, memory);
		cpu->rH = read_next_byte(cpu, memory);
		break;
	case 0x28:
		// JR Z, e8
		jr_conditional(cpu, memory, ZERO_FLAG, 1);
		break;
	case 0x31:
		// LD SP, n16
		cpu->sp = read_next_byte(cpu, memory) << 8;
		cpu->sp |= read_next_byte(cpu, memory);
		break;
	case 0x32:
		// LD [HL-], A
		uint16_t hl_val = bytes_to_u16(cpu->rL, cpu->rH);

		// Write value of regA to address stored in regHL
		write_to_bus(memory, hl_val, cpu->rA);

		hl_val--;
		
		cpu->rL = (uint8_t)(hl_val & 0xFF);
		cpu->rH = (uint8_t)(hl_val >> 8);
		
		break;
	case 0x60:
		// LD H, B
		cpu->rH = cpu->rB;
		break;
	case 0x61:
		// LD H, C
		cpu->rH = cpu->rC;
		break;
	case 0xAF:
		// XOR A, A (Clear accumulator)
		cpu->rA ^= cpu->rA;
		set_bit_u8(&cpu->rF, ZERO_FLAG, true);
		break;
	case 0xC3:
		// JP a16
		cpu->pc = read_next_u16(cpu, memory);
		break;
	case 0xDF:
		// RST 0x18
		push_to_stack(cpu, memory, cpu->pc);
		cpu->pc = 0x0018;
		break;
	case 0xFE:
		// CP A, n8
		uint8_t i_value = read_next_byte(cpu, memory);

		set_bit_u8(&cpu->rF, ZERO_FLAG, (cpu->rA == i_value));
		set_bit_u8(&cpu->rF, SUBTRACTION_FLAG, true);
		set_bit_u8(&cpu->rF, HALF_CARRY_FLAG, ((cpu->rA & 0x0F) < (i_value & 0x0F)));
		set_bit_u8(&cpu->rF, CARRY_FLAG, (cpu->rA < i_value));

		break;
	default:
		cpu->is_running = false;
		printf("Was unable to process instruction 0x%02X\n", op_code);
		break;
	}

	return op_code;
}