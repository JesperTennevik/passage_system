#ifndef STATE_ACCESS_GRANTED_H
#define STATE_ACCESS_GRANTED_H

#include "millis.h"
#include <stdint.h>

typedef struct sm_access_granted_t {
    millis_t entered;
} sm_access_granted_t;

#define SM_ACCESS_GRANTED_TIMEOUT 5 * 1000

struct state_machine;
void sm_enter_access_granted(struct state_machine *sm);

#endif