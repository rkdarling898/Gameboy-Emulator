#pragma once

#include "common.h"

#define CARTRIDGE_TYPE_ADDR 0x0147
#define CGB_FLAG_ADDR 0x0143
#define DESTINATION_CODE_ADDR 0x014A
#define LICENSE_CODE_ADDR 0x0144
#define OLD_LICENSE_CODE_ADDR 0x014B
#define RAM_SIZE_ADDR 0x0149
#define ROM_SIZE_ADDR 0x0148
#define SGB_FLAG_ADDR 0x0146
#define TITLE_ADDR 0x0134
#define VERSION_ADDR 0x014C
#define CHECKSUM_ADDR 0x014D

typedef struct {
    char title[17];
    uint8_t cgb_f;
    char license_c[3];
    uint8_t sgb_f;
    uint8_t cartridge_t;
    uint8_t rom_size_c;
    uint8_t ram_size_c;
    uint8_t destination_c;
    uint8_t old_license_c;
    uint8_t version_num;
    uint8_t checksum;
} cartridge_header;

void print_c_header (cartridge_header *cart_h) {
    printf("Title: %s\n", cart_h->title);
    printf("CGB Flag: %2X\n", cart_h->cgb_f);
    printf("Licensee Code: %s\n", cart_h->license_c);
    printf("SGB Flag: %2X\n", cart_h->cartridge_t);
    printf("ROM Size: %2X\n", cart_h->rom_size_c);
    printf("RAM Size: %2X\n", cart_h->ram_size_c);
    printf("Destination Code: %2X\n", cart_h->destination_c);
    printf("Old Licensee Code: %2X\n", cart_h->old_license_c);
    printf("Version Number: %2X\n", cart_h->version_num);
    printf("Header Checksum: %2X\n", cart_h->checksum);
}

void store_c_header_data (uint8_t *memory, cartridge_header *cart_h) {
    uint8_t max_title_size = 16;
    uint8_t cgb_f = 0;
    char current_char;

    cgb_f = *(memory + CGB_FLAG_ADDR);

    if (cgb_f == 0x80 || cgb_f == 0xC0) {
        cart_h->cgb_f = cgb_f;
        max_title_size = 15;
    }

    // Loop title chars until null terminator
    memset(cart_h->title, '\0', sizeof(cart_h->title));

    for (int i = 0; i < max_title_size; i++) {
        current_char = *(memory + TITLE_ADDR + i);
        cart_h->title[i] = current_char;

        if (current_char == '\0') break;
    }

    // Extract license code
    cart_h->license_c[0] = *(memory + LICENSE_CODE_ADDR);
    cart_h->license_c[1] = *(memory + LICENSE_CODE_ADDR + 1);
    cart_h->license_c[2] = '\0';

    cart_h->sgb_f = *(memory + SGB_FLAG_ADDR);
    cart_h->cartridge_t = *(memory + CARTRIDGE_TYPE_ADDR);
    cart_h->rom_size_c = *(memory + ROM_SIZE_ADDR);
    cart_h->ram_size_c = *(memory + RAM_SIZE_ADDR);
    cart_h->destination_c = *(memory + DESTINATION_CODE_ADDR);
    cart_h->old_license_c = *(memory + OLD_LICENSE_CODE_ADDR);
    cart_h->version_num = *(memory + VERSION_ADDR);
    cart_h->checksum = *(memory + CHECKSUM_ADDR);
}