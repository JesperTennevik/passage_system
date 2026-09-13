#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "sm_shared.h"
#include "state_input_await.h"
#include "state_error_blink.h"
#include "state_access_denied.h"
#include "state_access_granted.h"
#include "uart_protocol.h"
#include "mfrc522.h"

typedef struct sm_session_info {
    char uid[48];
    uint16_t sid;
    access_msg_t last_msg;
    access_status_t response_status;
    access_status_t send_status;
    millis_t send_time;
} sm_session_info;

typedef struct state_machine {
    state_e state;
    struct sm_input_await_t input_await;
    struct sm_error_blink_t error_blink;
    sm_session_info session;
    sm_access_denied_t access_denied;
    sm_access_granted_t access_granted;
} state_machine;

#define SM_RESPONSE_TIME 10 * 1000

void state_machine_init(state_machine *sm);
void sm_process_event(state_machine *sm, state_event_e event);

void sm_store_uid(const mfrc522_uid_t *uid, state_machine *sm);
state_event_e sm_parse_protocol_response(state_machine *sm);

#endif