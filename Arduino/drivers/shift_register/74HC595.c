#include "74HC595.h"
#include "spi.h"
#include <avr/io.h>
#include <util/delay.h>

static volatile uint8_t sr_reg;

#define SR_RCLK_DDR DDRD
#define SR_RCLK_PORT PORTD
#define SR_RCLK_PIN PD2

void sr_init(){
    SR_RCLK_DDR |= (1 << SR_RCLK_PIN);
    _delay_us(10);
    SR_RCLK_PORT &= ~(1 << SR_RCLK_PIN);
    _delay_us(10);
    sr_reg = 0b00000000;
    sr_set_mask(sr_reg);
}

void sr_set_bit_high(uint8_t bit){
    sr_reg |= (1 << bit);
    sr_set_mask(sr_reg);
}

void sr_set_bit_low(uint8_t bit){
    sr_reg &= ~(1 << bit);
    sr_set_mask(sr_reg);
}

void sr_toggle_bit(uint8_t bit){
    sr_reg ^= (1 << bit);
    sr_set_mask(sr_reg);
}

void sr_set_mask(uint8_t mask){
    sr_reg = mask;
    SR_RCLK_PORT &= ~(1 << SR_RCLK_PIN);
    _delay_us(10);
    spi_transfer(sr_reg);
    _delay_us(10);
    SR_RCLK_PORT |= (1 << SR_RCLK_PIN);
}

void sr_set_mask_high(uint8_t mask){
    sr_reg |= mask;
    sr_set_mask(sr_reg);
}

void sr_set_mask_low(uint8_t mask){
    sr_reg &= ~mask;
    sr_set_mask(sr_reg);
}

uint8_t sr_get_reg(){
    return sr_reg;
}

void sr_shift_right(){
    sr_reg = (sr_reg >> 1);
    sr_set_mask(sr_reg);
}

void sr_shift_left(){
    sr_reg = (sr_reg << 1);
    sr_set_mask(sr_reg);
}