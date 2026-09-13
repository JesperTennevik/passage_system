#include "state_access_granted.h"
#include "state_machine.h"
#include "uart.h"
#include "millis.h"
#include "rgb.h"
#include "SG90.h"


void sm_enter_access_granted(state_machine *sm){
    uart_write_string("STATE: Access Granted\r\n");

    rgb_off();
    rgb_set_green();

    servo_open();

    sm->access_granted.entered = millis_get();
}
