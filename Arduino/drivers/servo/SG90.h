#ifndef SG90_H
#define SG90_H

#include <stdint.h>

void servo_init();
void servo_open();
void servo_close();
void servo_set_tick(uint16_t ticks);

#endif