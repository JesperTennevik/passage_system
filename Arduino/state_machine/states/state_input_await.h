#ifndef STATE_INPUT_AWAIT_H
#define STATE_INPUT_AWAIT_H

#include <stdint.h>
#include "millis.h"

struct sm_input_await_t {
    millis_t entered;
    millis_t last_yellow_blink;
    millis_t last_input_blink;
    uint8_t yellow_blink_state;
    uint8_t input_idx;
    char input_pin[5];
};

#define SM_INPUT_AWAIT_TIMEOUT 10 * 1000
#define SM_INPUT_AWAIT_YELLOW_TIMER 500
#define SM_INPUT_AWAIT_INPUT_TIMER 100

struct state_machine;
void sm_enter_input_await(struct state_machine *sm);

#endif