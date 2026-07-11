#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "millis.h"
#include "uart.h"
#include "uart_protocol.h"
#include "mfrc522.h"

#define blink_timer 250

int main(void){
    millis_init();
    uart_init(BAUD_RATE);
    sei();

    DDRD |= (1 << PD3);
    PORTD |= (1 << PD3);
    
    millis_t last_blink = millis_get();
    while(1){
        millis_t now = millis_get();
        if(now - last_blink >= blink_timer){
            PORTD ^= (1 << PD3);
            last_blink = now;
            uart_write_string("Blink\r\n");
        }
    }
    return 0;
}