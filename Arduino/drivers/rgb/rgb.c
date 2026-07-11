#include "rgb.h"

#define RGB_RED 5
#define RGB_GREEN 6
#define RGB_BLUE 7

void rgb_init(){
    rgb_off();
}

void rgb_red_high(){
    sr_set_bit_high(RGB_RED);
}

void rgb_red_low(){
    sr_set_bit_low(RGB_RED);
}

void rgb_red_toggle(){
    sr_toggle_bit(RGB_RED);
}

void rgb_green_high(){
    sr_set_bit_high(RGB_GREEN);
}

void rgb_green_low(){
    sr_set_bit_low(RGB_GREEN);
}

void rgb_green_toggle(){
    sr_toggle_bit(RGB_GREEN);
}

void rgb_blue_high(){
    sr_set_bit_high(RGB_BLUE);
}

void rgb_blue_low(){
    sr_set_bit_low(RGB_BLUE);
}

void rgb_blue_toggle(){
    sr_toggle_bit(RGB_BLUE);
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
    sr_set_mask_high( (1 << RGB_RED) | (1 << RGB_GREEN) );
}
void rgb_set_magenta(){
    rgb_off();
    sr_set_mask_high( (1 << RGB_RED) | (1 << RGB_BLUE) );

}
void rgb_set_cyan(){
    rgb_off();
    sr_set_mask_high( (1 << RGB_GREEN) | (1 << RGB_BLUE) );
}

void rgb_off(){
    sr_set_mask_low( (1 << RGB_RED) | (1 << RGB_GREEN) | (1 << RGB_BLUE) );
}

void rgb_on(){
    sr_set_mask_high( (1 << RGB_RED) | (1 << RGB_GREEN) | (1 << RGB_BLUE) );
}