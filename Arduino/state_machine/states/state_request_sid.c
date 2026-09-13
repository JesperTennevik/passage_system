#include "state_request_sid.h"
#include "state_machine.h"
#include "uart.h"

void sm_enter_request_sid(state_machine *sm){
    uart_write_string("STATE: Request Sid\r\n");

    sm->session.response_status = ACCESS_CMD_UNKNOWN;
    sm->session.send_status = ACCESS_CMD_UNKNOWN;
    sm->session.sid = 0;
    sm->session.send_time = 0;
}