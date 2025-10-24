#include <stdio.h>

#include "common.h"
#include "cartridge_header.h"
#include "sm83.h"

#define MEMORY_MAX 8388608
#define ROM_GB 1
#define ROM_GB_COLOR 2

uint8_t gb_rom_type (char *filePath) {
    int len = strlen(filePath);
    char *ext2 = (char *)(filePath + len - 3);
    char *ext3 = (char *)(filePath + len - 4);

    if (strcmp(ext2, ".gb") == 0) {
        return ROM_GB;
    } else if (strcmp(ext3, ".gbc") == 0) {
        return ROM_GB_COLOR;
    }

    return 0;
}

void error (const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_usage (const char *program_name) {
    // Just setting this up to potentially take some options and flags later on
    printf("%s%s%s", "Usage: ", program_name, " (file.gb / file.gbc) [-o] [-f]\n");
    exit(EXIT_SUCCESS);
}

void print_cpu_state (sm83_ctx *cpu) {
    printf("\nPC: 0x%04X\n", cpu->pc);
    printf("SP: 0x%04X\n", cpu->sp);

    printf("A: %d\n", cpu->rA);
    printf("B: %d\n", cpu->rB);
    printf("C: %d\n", cpu->rC);
    printf("D: %d\n", cpu->rD);
    printf("E: %d\n", cpu->rE);
    printf("H: %d\n", cpu->rH);
    printf("L: %d\n", cpu->rL);
    
    printf("AF: %d\n", bytes_to_u16(cpu->rF, cpu->rA));
    printf("BC: %d\n", bytes_to_u16(cpu->rC, cpu->rB));
    printf("DE: %d\n", bytes_to_u16(cpu->rE, cpu->rD));
    printf("HL: %d (0x%04X)\n", bytes_to_u16(cpu->rL, cpu->rH), bytes_to_u16(cpu->rL, cpu->rH));

    printf("Zero Flag: %d\n", get_bit_u8(&cpu->rF, ZERO_FLAG));
    printf("Subtraction Flag: %d\n", get_bit_u8(&cpu->rF, SUBTRACTION_FLAG));
    printf("Half Carry Flag: %d\n", get_bit_u8(&cpu->rF, HALF_CARRY_FLAG));
    printf("Carry Flag: %d\n\n", get_bit_u8(&cpu->rF, CARRY_FLAG));

}

int main (int argc, char *argv[]) {
    sm83_ctx cpu = {0};
    cartridge_header cart_h = {0};
    uint8_t rom_type = 0;
    uint8_t *memory = NULL;
    size_t rom_size = 0;
    FILE *file = NULL;

    if (argc == 1)
        print_usage(argv[0]);

    if ((rom_type = gb_rom_type(argv[1])) == 0)
        print_usage(argv[0]);

    if ((file = fopen(argv[1], "rb")) == NULL)
        error("Unable to open file provided");

    if (fseek(file, 0, SEEK_END) < 0 || (rom_size = ftell(file)) < 0)
        error("Error occured getting ROM size");

    memory = (uint8_t *)malloc(rom_size);

    rewind(file);
    fread(memory, 1, rom_size, file);

    store_c_header_data(memory, &cart_h);

    cpu.sp = 0xFFFE;
    cpu.is_running = true;

    // Main Loop
    while (cpu.is_running) {
        //next_instruction(&cpu, memory);
        printf("%04X: 0x%02X\n", cpu.pc, memory[cpu.pc]);
        
        while (true) {
            char key = (char)getchar();

            if (key == 'c') {
                next_instruction(&cpu, memory);
                break;
            } else if (key == 'p') {
                print_cpu_state(&cpu);
            }
        }
    }

    free(memory);

    return EXIT_SUCCESS;
}