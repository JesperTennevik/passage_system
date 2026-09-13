#include "state_error_blink.h"
#include "state_machine.h"
#include "uart.h"
#include "rgb.h"
#include "millis.h"

void sm_enter_error_blink(state_machine *sm){
    uart_write_string("STATE: Error Blink\r\n");

    rgb_set_blue();

    sm->error_blink.entered = millis_get();
    sm->error_blink.last_blink = sm->error_blink.entered;
    sm->error_blink.blink_state = 1;
}