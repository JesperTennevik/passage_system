#pragma once

typedef struct {
    int sid;
    char command[16];
    char data[128];
} access_message_t;

bool parse_payload(char *payload, access_message_t *msg);
void handle_access_message(const access_message_t *msg);