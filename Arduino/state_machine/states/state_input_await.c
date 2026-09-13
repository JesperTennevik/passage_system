#include "state_input_await.h"
#include "state_machine.h"
#include "uart.h"
#include "rgb.h"

void sm_enter_input_await(state_machine *sm){
    uart_write_string("STATE: Input Await\r\n");

    rgb_off();

    sm->input_await.entered = millis_get();
    sm->input_await.last_yellow_blink = sm->input_await.entered;
    sm->input_await.last_input_blink = 0;
    sm->input_await.yellow_blink_state = 0;
    sm->input_await.input_idx = 0;
}