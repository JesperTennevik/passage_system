#define F_CPU 16000000UL
#define KEYPAD_MASK 0b00001111

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "keypad.h"
#include "74HC595.h"

/*
    4x4 keypad

    Layout:
    1 2 3 A
    4 5 6 B
    7 8 9 C
    * 0 # D
*/

static const char keypad_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void keypad_init(void){
    sr_set_mask_high(KEYPAD_MASK);

    DDRD &= ~((1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4));

    PORTD |= (1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4);
}

static void keypad_set_all_rows_high(void){
    sr_set_mask_high(KEYPAD_MASK);
}

static void keypad_set_row_low(uint8_t row){
    keypad_set_all_rows_high();

    sr_set_bit_low(row);
}

static uint8_t keypad_read_column(void){
    if (!(PIND & (1 << PD7))) return 0;
    if (!(PIND & (1 << PD6))) return 1;
    if (!(PIND & (1 << PD5))) return 2;
    if (!(PIND & (1 << PD4))) return 3;

    return 0xFF;
}

char keypad_get_key(void){
    uint8_t row;
    uint8_t col;

    for (row = 0; row < 4; row++){
        keypad_set_row_low(row);
        _delay_us(5);

        col = keypad_read_column();
        if (col != 0xFF){
            return keypad_map[row][col];
        }
    }

    return 0;
}

char keypad_get_key_debounced(void){
    char key = keypad_get_key();

    if (key != 0){
        _delay_ms(20);

        if (key == keypad_get_key()){
            while (keypad_get_key() != 0)
            {
                _delay_ms(5);
            }
            return key;
        }
    }

    return 0;
}