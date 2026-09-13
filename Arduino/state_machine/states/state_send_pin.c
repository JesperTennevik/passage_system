#include "state_machine.h"
#include "state_send_pin.h"
#include "uart.h"

void sm_enter_send_pin(state_machine *sm){
    uart_write_string("STATE: Send Pin\r\n");

    sm->session.response_status = ACCESS_CMD_UNKNOWN;
    sm->session.send_status = ACCESS_CMD_UNKNOWN;
    sm->session.send_time = 0;
}
