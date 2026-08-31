#include "rgb.h"
#include "74HC595.h"

#define RGB_PIN_RED 5
#define RGB_PIN_GREEN 6
#define RGB_PIN_BLUE 7

void rgb_init(){
    rgb_off();
}

void rgb_red_high(){
    sr_set_bit_high(RGB_PIN_RED);
}

void rgb_red_low(){
    sr_set_bit_low(RGB_PIN_RED);
}

void rgb_red_toggle(){
    sr_toggle_bit(RGB_PIN_RED);
}

void rgb_green_high(){
    sr_set_bit_high(RGB_PIN_GREEN);
}

void rgb_green_low(){
    sr_set_bit_low(RGB_PIN_GREEN);
}

void rgb_green_toggle(){
    sr_toggle_bit(RGB_PIN_GREEN);
}

void rgb_blue_high(){
    sr_set_bit_high(RGB_PIN_BLUE);
}

void rgb_blue_low(){
    sr_set_bit_low(RGB_PIN_BLUE);
}

void rgb_blue_toggle(){
    sr_toggle_bit(RGB_PIN_BLUE);
}

void rgb_set_red(){
    rgb_off();
    rgb_red_high();
}

void rgb_set_green(){
    rgb_off();
    rgb_green_high();
}

void rgb_set_blue(){
    rgb_off();
    rgb_blue_high();
}

void rgb_set_yellow(){
    rgb_off();
    sr_set_mask_high( (1 << RGB_PIN_RED) | (1 << RGB_PIN_GREEN) );
}
void rgb_set_magenta(){
    rgb_off();
    sr_set_mask_high( (1 << RGB_PIN_RED) | (1 << RGB_PIN_BLUE) );

}
void rgb_set_cyan(){
    rgb_off();
    sr_set_mask_high( (1 << RGB_PIN_GREEN) | (1 << RGB_PIN_BLUE) );
}

void rgb_off(){
    sr_set_mask_low( (1 << RGB_PIN_RED) | (1 << RGB_PIN_GREEN) | (1 << RGB_PIN_BLUE) );
}

void rgb_on(){
    sr_set_mask_high( (1 << RGB_PIN_RED) | (1 << RGB_PIN_GREEN) | (1 << RGB_PIN_BLUE) );
}

void rgb_set_color(rgb_color_e color){
    switch(color){
        case RGB_RED:
            rgb_set_red();
            break;
        case RGB_GREEN:
            rgb_set_green();
            break;
        case RGB_BLUE:
            rgb_set_blue();
            break;
        case RGB_YELLOW:
            rgb_set_yellow();
            break;
        case RGB_MAGENTA:
            rgb_set_magenta();
            break;
        case RGB_CYAN:
            rgb_set_cyan();
            break;
        case RGB_OFF:
            rgb_off();
            break;
    }
}