#ifndef RGB_H
#define RGB_H

typedef enum rgb_color_e{
    RGB_OFF,
    RGB_RED,
    RGB_GREEN,
    RGB_BLUE,
    RGB_YELLOW,
    RGB_MAGENTA,
    RGB_CYAN
} rgb_color_e;

void rgb_init();

void rgb_red_high();
void rgb_red_low();
void rgb_red_toggle();

void rgb_green_high();
void rgb_green_low();
void rgb_green_toggle();

void rgb_blue_high();
void rgb_blue_low();
void rgb_blue_toggle();

void rgb_set_red();
void rgb_set_green();
void rgb_set_blue();
void rgb_set_yellow();
void rgb_set_magenta();
void rgb_set_cyan();
void rgb_set_color(rgb_color_e color);

void rgb_off();
void rgb_on();

#endif