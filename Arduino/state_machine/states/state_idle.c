#include "state_idle.h"
#include "state_machine.h"
#include "uart.h"
#include "SG90.h"
#include "rgb.h"

void sm_enter_idle(state_machine *sm){
    uart_write_string("STATE: Idle\r\n");

    rgb_set_red();

    servo_close();
}