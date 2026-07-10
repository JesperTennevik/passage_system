#include "uart_protocol.h"
#include "uart_gateway.h"
#include "http_client.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "uart_protocol";

static bool is_hex_ascii_string(const char *s){
    if (s == NULL || s[0] == '\0') return false;
    for (size_t i = 0; s[i] != '\0'; i++) {
        bool ok = (s[i] >= '0' && s[i] <= '9') ||
                (s[i] >= 'A' && s[i] <= 'F') ||
                (s[i] >= 'a' && s[i] <= 'f');
        if (!ok) return false;
    }
    return true;
}

static void handle_uid(const access_message_t *msg){
    if (msg->sid != 0 || !is_hex_ascii_string(msg->data)) {
        uart_send_nack(msg->sid, "BAD_FORMAT");
        return;
    }
    
    
    char body[256];
    char response[256];
    int status_code = 0;
    
    // Expected: uid = uid|door_id
    char *uid;
    char *door_id;
    uid = msg->data;

    door_id = strchr(uid, '|');
    if(!uid) { return; }
    *door_id++ = '\0';

    snprintf(body, sizeof(body), "{\"uid\":\"%s\", \"door_id\":\"%s\"}", uid, door_id);

    esp_err_t err = http_post_json("/auth/uid", body, response, sizeof(response), &status_code);
    if (err != ESP_OK || status_code != 200) {
        uart_send_nack(msg->sid, "HTTP_ERROR");
        return;
    }

    api_response_t api = {0};
    if (!parse_api_response(response, &api)) {
        uart_send_nack(msg->sid, "BAD_API_RESPONSE");
        return;
    }

    uart_send_command(api.sid, api.status, "");
}

static void handle_pin(const access_message_t *msg){
    if (msg->sid <= 0 || msg->data[0] == '\0') {
        uart_send_nack(msg->sid, "BAD_FORMAT");
        return;
    }

    char body[256];
    char response[256];
    int status_code = 0;
    
    snprintf(body, sizeof(body), "{\"sid\":\"%d\",\"pin\":\"%s\"}", msg->sid, msg->data);

    esp_err_t err = http_post_json("/auth/pin", body, response, sizeof(response), &status_code);
    if (err != ESP_OK || status_code != 200) {
        uart_send_nack(msg->sid, "HTTP_ERROR");
        return;
    }

    api_response_t api = {0};
    if (!parse_api_response(response, &api)) {
        uart_send_nack(msg->sid, "BAD_API_RESPONSE");
        return;
    }

    uart_send_command(api.sid, api.status, "");
}

bool parse_payload(char *payload, access_message_t *msg)
{
    // Expected: SID|COMMAND|DATA
    char *saveptr = NULL;
    char *sid_str = strtok_r(NULL, "|", &saveptr);
    char *command = strtok_r(NULL, "|", &saveptr);
    char *data = strtok_r(NULL, "", &saveptr); // keep rest, may be empty

    if (sid_str == NULL || command == NULL || data == NULL) {
        return false;
    }

    char *endptr = NULL;
    long sid = strtol(sid_str, &endptr, 10);
    if (*endptr != '\0' || sid < 0 || sid > INT32_MAX) {
        return false;
    }

    msg->sid = (int) sid;
    strlcpy(msg->command, command, sizeof(msg->command));
    strlcpy(msg->data, data, sizeof(msg->data));
    return true;
}

void handle_access_message(const access_message_t *msg){
    ESP_LOGI(TAG, "CMD=%s SID=%d DATA=%s", msg->command, msg->sid, msg->data);

    if (strcmp(msg->command, "UID") == 0) {
        handle_uid(msg);
    } else if (strcmp(msg->command, "PIN") == 0) {
        handle_pin(msg);
    } else {
        uart_send_nack(msg->sid, "UNKNOWN_COMMAND");
    }
}