#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lfsr.h"

void lfsr_calculate(uint16_t *reg) {
    /* YOUR CODE HERE */
    uint16_t bit0 = *reg & 1u;
    uint16_t bit2 = (*reg >> 2) & 1u;
    uint16_t bit3 = (*reg >> 3) & 1u;
    uint16_t bit5 = (*reg >> 5) & 1u;
    uint16_t bit = bit0 ^ bit2 ^ bit3 ^ bit5;
    *reg = (*reg >> 1) | (bit << 15);
}

