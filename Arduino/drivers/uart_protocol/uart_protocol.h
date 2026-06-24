#ifndef ACCESS_PROTOCOL_H
#define ACCESS_PROTOCOL_H

#include <stdint.h>

#define ACCESS_STX 0x02
#define ACCESS_ETX 0x03

#define ACCESS_MAX_PAYLOAD 48
#define ACCESS_MAX_DATA 24 
#define ACCESS_TIMEOUT_MS 1000

typedef enum {
    ACCESS_STATUS_OK,
    ACCESS_STATUS_NO_DATA,  
    ACCESS_STATUS_BAD_LEN,
    ACCESS_STATUS_BAD_PAYLOAD,
    ACCESS_STATUS_BAD_FORMAT,
    ACCESS_STATUS_BAD_CHECKSUM,
    ACCESS_STATUS_BUFFER_TOO_SMALL,
    ACCESS_STATUS_TIMEOUT,
    ACCESS_STATUS_UNKNOWN_COMMAND
} access_status_t;

typedef enum {
    ACCESS_CMD_OK,
    ACCESS_CMD_UID,
    ACCESS_CMD_REQ_PIN,
    ACCESS_CMD_PIN,
    ACCESS_CMD_ERR,
    ACCESS_CMD_TIMEOUT,
    ACCESS_CMD_LOCKED,
    ACCESS_CMD_UNKNOWN
} access_cmd_t;

typedef struct {
    uint16_t sid;
    access_cmd_t cmd;
    char data[ACCESS_MAX_DATA];
} access_msg_t;

void access_init();

access_status_t access_send_uid(const char *uid);
access_status_t access_send_pin(uint16_t sid, const char *pin);
access_status_t access_read_msg(access_msg_t *msg);

#endif