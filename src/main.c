#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "millis.h"

#define blink_timer 250

int main(void){
    millis_init();
    sei();

    DDRD |= (1 << PD3);
    PORTD |= (1 << PD3);
    
    millis_t last_blink = millis_get();
    while(1){
        millis_t now = millis_get();
        if(now - last_blink >= blink_timer){
            PORTD ^= (1 << PD3);
            last_blink = now;
        }
    }
    return 0;
}