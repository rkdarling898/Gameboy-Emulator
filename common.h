#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void set_bit_u8 (uint8_t *target, uint8_t index, bool turn_bit_on) {
    if (index > 7) return;

    if (turn_bit_on) {
        *target |= 1 << index;
    } else {
        *target &= ((1 << index) ^ 255); // Right hand operand creates applicable mask
    }
}

uint16_t bytes_to_u16 (uint8_t low_byte, uint8_t high_byte) {
    return ((uint16_t)high_byte << 8) | low_byte;
}