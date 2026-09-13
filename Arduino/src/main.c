#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "millis.h"
#include "spi.h"
#include "74HC595.h"
#include "rgb.h"
#include "keypad.h"
#include "SG90.h"
#include "mfrc522.h"
#include "uart_protocol.h"
#include "uart.h"
#include "state_machine.h"

static state_machine sm;

#define DOOR_ID "test_door"

void run(){
    millis_t now = millis_get();

    if(sm.state == SM_STATE_IDLE){
        uint8_t atqa[2];
        uint8_t atqa_len = 2;
        mfrc522_uid_t uid;

        if (mfrc522_request_a(atqa, &atqa_len) == MFRC522_OK){
            if (mfrc522_anticoll_select(&uid) == MFRC522_OK){
                sm_store_uid(&uid, &sm);
                sm_process_event(&sm, SM_EVENT_RFID_SCANNED);
            }
        }

        char key = keypad_get_key_debounced();
        if (key == '*'){
            sm_process_event(&sm, SM_EVENT_RFID_SCANNED);
            return;
        }
    }

    if(sm.state == SM_STATE_INPUT_AWAIT){
        if(now - sm.input_await.entered >= SM_INPUT_AWAIT_TIMEOUT){
            sm_process_event(&sm, SM_EVENT_INPUT_TIMEOUT);
            return;
        }

        char key = keypad_get_key_debounced();
        if (key != 0){
            sm.input_await.last_input_blink = now;

            uart_write_string("\r\n");
            uart_write_char(key);

            sm.input_await.input_pin[sm.input_await.input_idx++] = key;
            char o[18];
            snprintf(o, sizeof(o), "\r\nIdx: %d\r\n", sm.input_await.input_idx);
            uart_write_string(o);

            if(sm.input_await.input_idx >= 4){
                sm.input_await.input_pin[4] = '\0';
                sm_process_event(&sm, SM_EVENT_PIN_INPUTTED);
                return;
            }
        }

        if(sm.input_await.last_input_blink != 0 &&
            now - sm.input_await.last_input_blink < SM_INPUT_AWAIT_INPUT_TIMER){
            rgb_set_color(RGB_CYAN);
        }
        else {
            if(now - sm.input_await.last_yellow_blink >= SM_INPUT_AWAIT_YELLOW_TIMER){
                sm.input_await.yellow_blink_state ^= 1;
                sm.input_await.last_yellow_blink = now;
            }

            sm.input_await.yellow_blink_state ? rgb_set_yellow() : rgb_off();
        }        
    }

    if(sm.state == SM_STATE_REQUEST_SID){
        if(sm.session.send_time == 0){
            sm.session.send_status = access_send_uid(sm.session.uid, DOOR_ID);
            sm.session.send_time = now;
        }

        if(now - sm.session.send_time >= SM_RESPONSE_TIME){
            sm_process_event(&sm, SM_EVENT_PROTOCOL_STATUS_TIMEOUT);
        }

        if(sm.session.send_status == ACCESS_STATUS_OK){
            sm.session.response_status = access_read_msg(&sm.session.last_msg);
            if(sm.session.response_status == ACCESS_STATUS_OK){
                sm_process_event(&sm, sm_parse_protocol_response(&sm));
            }
        } else {
            sm_process_event(&sm, SM_EVENT_PROTOCOL_STATUS_ERR);
        }
    }

    if(sm.state == SM_STATE_SEND_PIN){
        if(sm.session.send_time == 0){
            sm.session.send_status = access_send_pin(sm.session.sid, sm.input_await.input_pin);
            sm.session.send_time = now;
        }

        if(now - sm.session.send_time >= SM_RESPONSE_TIME){
            sm_process_event(&sm, SM_EVENT_PROTOCOL_STATUS_TIMEOUT);
        }

        if(sm.session.send_status == ACCESS_STATUS_OK){
            sm.session.response_status = access_read_msg(&sm.session.last_msg);
            if(sm.session.response_status == ACCESS_STATUS_OK){
                sm_process_event(&sm, sm_parse_protocol_response(&sm));
            }
        }
        else {
            sm_process_event(&sm, SM_EVENT_PROTOCOL_STATUS_ERR);
        }
    }

    if(sm.state == SM_STATE_ERROR_BLINK){
        if(now - sm.error_blink.last_blink >= SM_ERROR_BLINK_TIMER){
            sm.error_blink.blink_state ^= 1;
            sm.error_blink.last_blink = now;
        }

        sm.error_blink.blink_state ? rgb_set_blue() : rgb_off();

        if(now - sm.error_blink.entered >= SM_ERROR_BLINK_TIMEOUT){
            sm_process_event(&sm, SM_EVENT_ERROR_BLINK_TIMEOUT);
        }
    }

    if(sm.state == SM_STATE_ACCESS_GRANTED){
        if(now - sm.access_granted.entered >= SM_ACCESS_GRANTED_TIMEOUT){
            sm_process_event(&sm, SM_EVENT_SESSION_EXPIRED);
        }
    }

    if(sm.state == SM_STATE_ACCESS_DENIED){
        if(now - sm.access_denied.entered >= SM_ACCESS_DENIED_TIMEOUT){
            sm_process_event(&sm, SM_EVENT_SESSION_EXPIRED);
        }

        if(now - sm.access_denied.last_blink >= SM_ACCESS_DENIED_BLINK_TIMER){
            sm.access_denied.blink_state ^= 1;
            sm.access_denied.last_blink = now;
        }

        sm.access_denied.blink_state ? rgb_set_red() : rgb_off();
    }
}

int main(void){
    millis_init();
    uart_init(BAUD_RATE);
    spi_init(); 
    sr_init();
    keypad_init();
    rgb_init();
    servo_init();
    mfrc522_init();
    state_machine_init(&sm);

    sei(); 

    uart_write_string("System ready: \n");
    
    while(1){
        run();
    }
    return 0;
}