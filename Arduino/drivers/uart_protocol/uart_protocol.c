#include "uart_protocol.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "uart.h"
#include "millis.h"

static const char CMD_UID[] PROGMEM = "UID";
static const char CMD_PIN[] PROGMEM = "PIN";
static const char CMD_NACK[] PROGMEM = "NACK";

static const char NACK_BAD_CHECKSUM[] PROGMEM = "BAD_CHECKSUM";
static const char NACK_BAD_LEN[] PROGMEM = "BAD_LEN";
static const char NACK_BAD_FORMAT[] PROGMEM = "BAD_FORMAT";
static const char NACK_TIMEOUT[] PROGMEM = "TIMEOUT";
static const char NACK_UNKNOWN_CMD[] PROGMEM = "UNKNOWN_CMD";

typedef enum {
    RX_WAIT_STX,
    RX_WAIT_LEN,
    RX_READ_PAYLOAD,
    RX_WAIT_CHECKSUM,
    RX_WAIT_ETX
} rx_state_t;

typedef struct {
    rx_state_t state;
    uint8_t len;
    uint8_t idx;
    uint8_t checksum;
    millis_t last_byte_time;
    char payload[ACCESS_MAX_PAYLOAD + 1];
} rx_ctx_t;

static rx_ctx_t rx_ctx;

static void rx_reset(){
    rx_ctx.state = RX_WAIT_STX;
    rx_ctx.len = 0;
    rx_ctx.idx = 0;
    rx_ctx.checksum = 0;
    rx_ctx.last_byte_time = 0;
}

static uint8_t strlen_progmem_uint8(const char *str){
    uint8_t len = 0;

    while (pgm_read_byte(str++) != '\0'){
        len++;
    }
    return len;
}

static uint8_t uint16_char_len(uint16_t val){
    if(val >= 10000){ return 5; }
    if(val >= 1000) { return 4; }
    if(val >= 100)  { return 3; }
    if(val >= 10)   { return 2; }
    return 1;
}

static uint8_t xor_ascii_uint16(uint16_t val){
    uint8_t checksum = 0;
    char buff[6];
    itoa(val, buff, 10);

    for(uint8_t i = 0; buff[i] != '\0'; i++){
        checksum ^= buff[i];
    }
    return checksum;
}

static uint8_t xor_sram(const char *str){
    uint8_t checksum = 0;

    while(*str){
        checksum ^= (uint8_t)*str++;
    }
    return checksum;
}

static uint8_t xor_progmem(const char *str){
    uint8_t checksum = 0;
    char c;

    while((c = pgm_read_byte(str++)) != '\0'){
        checksum ^= (uint8_t)c;
    }
    return checksum;
}

static void access_uart_write_progmem(const char *str){
    char c;
    while((c = pgm_read_byte(str++))){
        uart_write_char(c);
    }
}

static access_status_t send_payload(uint16_t sid, const char *cmd, const char *data){
    uint8_t len = 0;
    uint8_t checksum = 0;

    if(data == NULL){
        data = "";
    }

    // sid|cmd|data
    len += 2;                       // '|' separator * 2
    len += uint16_char_len(sid);
    len += strlen_progmem_uint8(cmd);
    len += (uint8_t)strlen(data);

    if(len > ACCESS_MAX_PAYLOAD){
        return ACCESS_STATUS_BUFFER_TOO_SMALL;
    }

    checksum ^= xor_ascii_uint16(sid);
    checksum ^= '|';
    checksum ^= xor_progmem(cmd);
    checksum ^= '|';
    checksum ^= xor_sram(data);

    uart_write_char((char)ACCESS_STX);
    uart_write_char((char)len);

    char buff[6];
    itoa(sid, buff, 10);
    uart_write_string(buff);

    uart_write_char('|');
    access_uart_write_progmem(cmd);
    uart_write_char('|');

    while(*data){
        uart_write_char(*data++);
    }

    uart_write_char((char)checksum);
    uart_write_char((char)ACCESS_ETX);

    return ACCESS_STATUS_OK;
}

access_status_t access_send_uid(const char *uid, const char *door_id){
    char data[32];
    snprintf(data, sizeof(data), "%s|%s", uid, door_id);

    return send_payload(0, CMD_UID, data);
}

access_status_t access_send_pin(uint16_t sid, const char *pin){
    return send_payload(sid, CMD_PIN, pin);
}

static bool is_valid_payload_char(const char c){
    return c >= 32 && c <= 126;
}

static bool parse_sid(const char *sid_str, uint16_t *sid){
    uint32_t val;

    if(*sid_str == '\0'){ return false; }

    char *end;
    val = strtoul(sid_str, &end, 10);
    if(val > 65535ul || *end != '\0'){ return false; }

    *sid = (uint16_t)val;
    return true;
}

static access_cmd_t parse_cmd(const char *cmd_str){
    if(strcmp(cmd_str, "OK") == 0)      { return ACCESS_CMD_OK; }
    if(strcmp(cmd_str, "UID") == 0)     { return ACCESS_CMD_UID; }
    if(strcmp(cmd_str, "REQ_PIN") == 0) { return ACCESS_CMD_REQ_PIN; }
    if(strcmp(cmd_str, "PIN") == 0)     { return ACCESS_CMD_PIN; }
    if(strcmp(cmd_str, "ERR") == 0)     { return ACCESS_CMD_ERR; }
    if(strcmp(cmd_str, "TIMEOUT") == 0) { return ACCESS_CMD_TIMEOUT; }
    if(strcmp(cmd_str, "LOCKED") == 0)  { return ACCESS_CMD_LOCKED; }
    if(strcmp(cmd_str, "NACK") == 0)  { return ACCESS_CMD_NACK; }
    
    return ACCESS_CMD_UNKNOWN;
}

static access_status_t send_progmem_payload(uint16_t sid, const char *cmd, const char *data){
    uint8_t len = 0;
    uint8_t checksum = 0;

    if(data == NULL){
        data = "";
    }

    // sid|cmd|data
    len += 2;                       // '|' separator * 2
    len += uint16_char_len(sid);
    len += strlen_progmem_uint8(cmd);
    len += strlen_progmem_uint8(data);

    if(len > ACCESS_MAX_PAYLOAD){
        return ACCESS_STATUS_BUFFER_TOO_SMALL;
    }

    checksum ^= xor_ascii_uint16(sid);
    checksum ^= '|';
    checksum ^= xor_progmem(cmd);
    checksum ^= '|';
    checksum ^= xor_progmem(data);

    uart_write_char((char)ACCESS_STX);
    uart_write_char((char)len);

    char buff[6];
    itoa(sid, buff, 10);
    uart_write_string(buff);

    uart_write_char('|');
    access_uart_write_progmem(cmd);
    uart_write_char('|');

    access_uart_write_progmem(data);

    uart_write_char((char)checksum);
    uart_write_char((char)ACCESS_ETX);

    return ACCESS_STATUS_OK;
}

static access_status_t parse_payload(access_msg_t *msg){
    char *sid_str;
    char *cmd_str;
    char *data_str;
    access_cmd_t cmd;

    sid_str = rx_ctx.payload;

    cmd_str = strchr(sid_str, '|');
    if(!cmd_str) { return ACCESS_STATUS_BAD_FORMAT; }
    *cmd_str++ = '\0';

    data_str = strchr(cmd_str, '|');
    if(!data_str) { return ACCESS_STATUS_BAD_FORMAT; }
    *data_str++ = '\0';

    if(!parse_sid(sid_str, &msg->sid)){ return ACCESS_STATUS_BAD_FORMAT; }

    cmd = parse_cmd(cmd_str);
    
    msg->cmd = cmd;
    
    if(cmd == ACCESS_CMD_UNKNOWN){
        msg->data[0] = '\0';
        return ACCESS_STATUS_UNKNOWN_COMMAND;
    }

    strncpy(msg->data, data_str, ACCESS_MAX_DATA - 1);
    msg->data[ACCESS_MAX_DATA - 1] = '\0';

    return ACCESS_STATUS_OK;
}

access_status_t access_read_msg(access_msg_t *msg){
    char c;
    millis_t now = millis_get();

    if(rx_ctx.state != RX_WAIT_STX && rx_ctx.last_byte_time != 0 && now - rx_ctx.last_byte_time > ACCESS_TIMEOUT_MS){
        rx_reset();
        send_progmem_payload(0, CMD_NACK, NACK_TIMEOUT);
        return ACCESS_STATUS_TIMEOUT;
    }

    while(uart_available()){
        if(!uart_read_char(&c)){
            return ACCESS_STATUS_NO_DATA;
        }

        rx_ctx.last_byte_time = millis_get();

        if((uint8_t)c == ACCESS_STX){
            rx_reset();
            rx_ctx.state = RX_WAIT_LEN;
            continue;
        }

        switch(rx_ctx.state){
            case RX_WAIT_STX:
                break;

            case RX_WAIT_LEN:
                rx_ctx.len = (uint8_t)c;
                
                if(rx_ctx.len == 0 || rx_ctx.len > ACCESS_MAX_PAYLOAD){
                    rx_reset();
                    send_progmem_payload(0, CMD_NACK, NACK_BAD_LEN);
                    return ACCESS_STATUS_BAD_LEN;
                }

                rx_ctx.state = RX_READ_PAYLOAD;
                break;

            case RX_READ_PAYLOAD:
                if(!is_valid_payload_char(c)){
                    send_progmem_payload(0, CMD_NACK, NACK_BAD_FORMAT);
                    return ACCESS_STATUS_BAD_PAYLOAD;
                }

                rx_ctx.payload[rx_ctx.idx++] = c;
                rx_ctx.checksum ^= (uint8_t)c;
                if(rx_ctx.idx >= rx_ctx.len){
                    rx_ctx.payload[rx_ctx.idx] = '\0';
                    rx_ctx.state = RX_WAIT_CHECKSUM;
                }

                break;

            case RX_WAIT_CHECKSUM:
                if(rx_ctx.checksum != (uint8_t)c){
                    rx_reset();
                    send_progmem_payload(0, CMD_NACK, NACK_BAD_CHECKSUM);
                    return ACCESS_STATUS_BAD_CHECKSUM;
                }

                rx_ctx.state = RX_WAIT_ETX;
                break;

            case RX_WAIT_ETX:
                if((uint8_t)c != ACCESS_ETX){
                    rx_reset();
                    send_progmem_payload(0, CMD_NACK, NACK_BAD_FORMAT);
                    return ACCESS_STATUS_BAD_FORMAT;
                }

                rx_reset();
                {
                    access_status_t status = parse_payload(msg);

                    switch(status){
                        case ACCESS_CMD_OK:
                            break;
                        case ACCESS_STATUS_UNKNOWN_COMMAND:
                            send_progmem_payload(msg->sid, CMD_NACK, NACK_UNKNOWN_CMD);
                            break; 
                        default:
                            send_progmem_payload(msg->sid, CMD_NACK, NACK_BAD_FORMAT);
                    }
                    return status;
                }
        }
    }
    
    return ACCESS_STATUS_NO_DATA;
}

void access_init(){
    rx_reset();
}