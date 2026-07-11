#ifndef SR_74HC595_H
#define SR_74HC595_H

#include <stdint.h>

void sr_init();
void sr_set_bit_high(uint8_t bit);
void sr_set_bit_low(uint8_t bit);
void sr_toggle_bit(uint8_t bit);
void sr_set_mask(uint8_t mask);
void sr_set_mask_high(uint8_t mask);
void sr_set_mask_low(uint8_t mask);
void sr_shift_left();
void sr_shift_right();
uint8_t sr_get_reg();

#endif