#pragma once
#include "esp_err.h"

typedef struct {
    char status[16];
    int sid;
} api_response_t;

bool parse_api_response(const char *json, api_response_t *out);

esp_err_t http_post_json(
    const char *endpoint,
    const char *json_body,
    char *response_buf,
    size_t response_buf_size,
    int *status_code
);
