#include "SG90.h"
#include <avr/io.h>

#define SERVO_LEFT 4000
#define SERVO_MIDDLE 3000
#define SERVO_RIGHT 2000

// range = 912 - 4976 -- Wokwi
// range = 694 - 5262 -- real world SG90

void servo_init(){
    DDRB |= (1 << PB1);
    
    TCCR1A |= (1 << COM1A1);

    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);

    TCCR1B |= (1 << CS11);

    ICR1 = 40000;
}

void servo_set_angle(uint16_t angle){
    OCR1A = angle;
}

void servo_open(){
    servo_set_angle(SERVO_RIGHT);
}

void servo_close(){
    servo_set_angle(SERVO_LEFT);
}

void servo_set_tick(uint16_t ticks){
    servo_set_angle(ticks);
}