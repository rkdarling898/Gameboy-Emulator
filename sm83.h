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

uint8_t next_instruction (sm83_ctx *cpu, uint8_t *memory) {
	uint8_t op_code = *(memory + cpu->pc);

	switch (op_code) {
	case 0x00:
		break;
	case 0x21:
		// LD [HL+], A
		break;
	case 0xAF:
		cpu->rA ^= cpu->rA;
		
		if (cpu->rA == 0) {
			set_bit_u8(&cpu->rA, ZERO_FLAG, true);
		} else {
			set_bit_u8(&cpu->rA, ZERO_FLAG, false);
		}

		cpu->pc++;

		break;
	case 0xC3:
		cpu->pc = bytes_to_u16(*(memory + cpu->pc + 1), *(memory + cpu->pc + 2));
		break;
	default:
		cpu->is_running = false;
		printf("Was unable to process instruction 0x%2X\n", op_code);
		break;
	}

	return op_code;
}