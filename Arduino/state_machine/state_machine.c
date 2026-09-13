#include "state_machine.h"
#include "state_idle.h"
#include "state_error_blink.h"
#include "state_request_sid.h"
#include "state_send_pin.h"
#include "state_access_denied.h"
#include "state_access_granted.h"
#include "mfrc522.h"
#include <stdio.h>
#include <stdint.h>

static void enter_state(state_machine *sm, const state_transition *transition){

    sm->state = transition->to;

    switch(transition->to){
        case SM_STATE_IDLE:
            sm_enter_idle(sm);
            break;
        case SM_STATE_INPUT_AWAIT:
            sm_enter_input_await(sm);
            break;
        case SM_STATE_ACCESS_GRANTED:
            sm_enter_access_granted(sm);
            break;
        case SM_STATE_ACCESS_DENIED:
            sm_enter_access_denied(sm);
            break;
        case SM_STATE_ERROR_BLINK:
            sm_enter_error_blink(sm);
            break;
        case SM_STATE_REQUEST_SID:
            sm_enter_request_sid(sm);
            break;
        case SM_STATE_SEND_PIN:
            sm_enter_send_pin(sm);
            break;
    }
}

void state_machine_init(state_machine *sm){
    sm->state = SM_STATE_IDLE;
    sm_process_event(sm, SM_EVENT_BOOT);
}

void sm_process_event(state_machine *sm, const state_event_e event){
    for(uint8_t i = 0; i < (sizeof(state_transitions) / sizeof(state_transitions[0])); i++){
        if(sm->state == state_transitions[i].from && event == state_transitions[i].event){
            enter_state(sm, &state_transitions[i]);
            return;
        }
    }
}

static void uid_to_str(const mfrc522_uid_t *uid, char *out)
{
    for (uint8_t i = 0; i < uid->size; i++) {
        sprintf(&out[i * 2], "%02X", uid->uid[i]);
    }
    out[uid->size * 2] = '\0';
}

void sm_store_uid(const mfrc522_uid_t *mfrc_uid, state_machine *sm){
    uid_to_str(mfrc_uid, sm->session.uid);
}

state_event_e sm_parse_protocol_response(state_machine *sm){
    switch(sm->session.last_msg.cmd){
        case ACCESS_CMD_OK:
            return SM_EVENT_PROTOCOL_CMD_OK;
        case ACCESS_CMD_REQ_PIN:
            sm->session.sid = sm->session.last_msg.sid;
            return SM_EVENT_PROTOCOL_CMD_REQ_PIN;
        case ACCESS_CMD_ERR:
            return SM_EVENT_PROTOCOL_CMD_ERR;
        case ACCESS_CMD_LOCKED:
            return SM_EVENT_PROTOCOL_CMD_ERR;
        case ACCESS_CMD_TIMEOUT:
            return SM_EVENT_PROTOCOL_CMD_ERR;
        case ACCESS_CMD_NACK:
            return SM_EVENT_PROTOCOL_CMD_ERR;
        default:
            return SM_EVENT_PROTOCOL_CMD_UNEXPECTED;
    }
}
