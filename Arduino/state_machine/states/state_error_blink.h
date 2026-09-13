#ifndef STATE_ERROR_BLINK_H
#define STATE_ERROR_BLINK_H

#include "millis.h"
#include <stdint.h>

struct sm_error_blink_t {
    millis_t entered;
    millis_t last_blink;
    uint8_t blink_state;
};

#define SM_ERROR_BLINK_TIMEOUT 2 * 1000
#define SM_ERROR_BLINK_TIMER 100

struct state_machine;
void sm_enter_error_blink(struct state_machine *sm);

#endif