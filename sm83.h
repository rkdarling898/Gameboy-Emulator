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

uint8_t read_from_bus (uint8_t *memory, uint16_t addr) {
	// This will be greatly expanded and error checked later
	return *(memory + addr);
}

void write_to_bus (uint8_t *memory, uint16_t addr, uint8_t data) {
	// This will also be worked on
	*(memory + addr) = data;
}

void ld_next_byte (sm83_ctx *cpu, uint8_t *memory, uint8_t *dest) {
	cpu->pc++;
	*dest = read_from_bus(memory, cpu->pc++);
}

void push_to_stack (sm83_ctx *cpu, uint8_t *memory, uint16_t data) {
	uint8_t low_byte = (uint8_t)((data & 0xFF00) >> 8);
	uint8_t high_byte = (uint8_t)(data & 0x00FF);

	cpu->sp--;
	write_to_bus(memory, cpu->sp, high_byte);

	cpu->sp--;
	write_to_bus(memory, cpu->sp, low_byte);
}

uint8_t next_instruction (sm83_ctx *cpu, uint8_t *memory) {
	uint8_t op_code = *(memory + cpu->pc);

	switch (op_code) {
	case 0x00:
		// NOP
		break;
	case 0x06:
		// LD B, n8
		ld_next_byte(cpu, memory, &cpu->rB);
		break;
	case 0x0E:
		// LD C, n8
		ld_next_byte(cpu, memory, &cpu->rC);
		break;
	case 0x21:
		// LD HL, n16
		cpu->pc++;

		cpu->rL = read_from_bus(memory, cpu->pc++);
		cpu->rH = read_from_bus(memory, cpu->pc++);
		break;
	case 0x31:
		// LD SP, n16
		cpu->pc++;

		cpu->sp = read_from_bus(memory, cpu->pc++) << 8;
		cpu->sp |= read_from_bus(memory, cpu->pc++);
		break;
	case 0x32:
		// LD [HL-], A
		cpu->pc++;

		uint16_t hl_val = bytes_to_u16(cpu->rL, cpu->rH);

		write_to_bus(memory, hl_val, cpu->rA);

		hl_val--;
		
		cpu->rL = (uint8_t)((hl_val & 0xFF00) >> 8);
		cpu->rH = (uint8_t)(hl_val & 0x00FF);
		
		break;
	case 0xAF:
		// XOR A, A
		cpu->rA ^= cpu->rA;
		
		if (cpu->rA == 0) {
			set_bit_u8(&cpu->rA, ZERO_FLAG, true);
		} else {
			set_bit_u8(&cpu->rA, ZERO_FLAG, false);
		}

		cpu->pc++;
		break;
	case 0xC3:
		// JP a16
		cpu->pc = bytes_to_u16(*(memory + cpu->pc + 1), *(memory + cpu->pc + 2));
		break;
	case 0xDF:
		// RST 0x18
		cpu->pc++;
		push_to_stack(cpu, memory, cpu->pc);

		cpu->pc = 0x0018;
		break;
	default:
		cpu->is_running = false;
		printf("Was unable to process instruction 0x%02X\n", op_code);
		break;
	}

	return op_code;
}