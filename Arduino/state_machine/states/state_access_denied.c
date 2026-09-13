#include "state_access_denied.h"
#include "state_machine.h"
#include "uart.h"
#include "millis.h"
#include "rgb.h"
#include "SG90.h"

void sm_enter_access_denied(state_machine *sm){
    uart_write_string("STATE: Access Denied\r\n");

    rgb_off();

    servo_close();

    sm->access_denied.entered = millis_get();
    sm->access_denied.blink_state = 0;
    sm->access_denied.last_blink = sm->access_denied.entered;
}
