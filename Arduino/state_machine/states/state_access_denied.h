#ifndef STATE_ACCESS_DENIED_H
#define STATE_ACCESS_DENIED_H

#include "millis.h"
#include <stdint.h>

typedef struct sm_access_denied_t {
    millis_t entered;
    millis_t last_blink;
    uint8_t blink_state;
} sm_access_denied_t;

#define SM_ACCESS_DENIED_TIMEOUT 5 * 1000
#define SM_ACCESS_DENIED_BLINK_TIMER 500

struct state_machine;
void sm_enter_access_denied(struct state_machine *sm);

#endif