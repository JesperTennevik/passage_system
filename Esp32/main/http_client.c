#include "http_client.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"

#define ESP_API_BASE_URL CONFIG_ESP_API_BASE_URL
#define API_BASE_URL ESP_API_BASE_URL

static const char *TAG = "http_client";

typedef struct {
    char *buf;
    size_t size;
    size_t len;
} http_response_buffer_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt){
    if (evt->event_id == HTTP_EVENT_ON_DATA && evt->user_data && evt->data && evt->data_len > 0) {
        http_response_buffer_t *rb = (http_response_buffer_t *) evt->user_data;
        if (rb->len < rb->size - 1) {
            size_t copy_len = evt->data_len;
            size_t free_space = (rb->size - 1) - rb->len;
            if (copy_len > free_space) copy_len = free_space;
            memcpy(rb->buf + rb->len, evt->data, copy_len);
            rb->len += copy_len;
            rb->buf[rb->len] = '\0';
        }
    }
    return ESP_OK;
}

esp_err_t http_post_json(const char *endpoint, const char *json_body,
                                char *response_buf, size_t response_buf_size,
                                int *status_code)
{
    char url[160];
    snprintf(url, sizeof(url), "%s%s", API_BASE_URL, endpoint);

    if (response_buf_size > 0) response_buf[0] = '\0';
    http_response_buffer_t rb = {
        .buf = response_buf,
        .size = response_buf_size,
        .len = 0,
    };

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
        .event_handler = http_event_handler,
        .user_data = &rb,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        return ESP_FAIL;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_post_field(client, json_body, strlen(json_body));

    ESP_LOGI(TAG, "HTTP POST %s body: %s", endpoint, json_body);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int code = esp_http_client_get_status_code(client);
        int content_length = esp_http_client_get_content_length(client);
        if (status_code) *status_code = code;
        ESP_LOGI(TAG, "HTTP %s -> %d, content_length=%d, body_len=%d: %s",
                endpoint, code, content_length, (int) rb.len, response_buf);
    } else {
        ESP_LOGE(TAG, "HTTP POST %s failed: %s", endpoint, esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

bool parse_api_response(const char *json, api_response_t *out){
    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        return false;
    }

    const cJSON *status =
        cJSON_GetObjectItemCaseSensitive(root, "status");

    const cJSON *sid =
        cJSON_GetObjectItemCaseSensitive(root, "sid");

    const cJSON *err =
        cJSON_GetObjectItemCaseSensitive(root, "error");

    if (err != NULL){
        strlcpy(out->status, err->valuestring, sizeof(out->status));
        out->sid = -1;
        cJSON_Delete(root);
        return true;
    }

    if (!cJSON_IsString(status)) {
        cJSON_Delete(root);
        return false;
    }

    int sid_value = 0;

    if (cJSON_IsNumber(sid)) {
        sid_value = sid->valueint;
    }
    else if (cJSON_IsString(sid) && sid->valuestring != NULL) {
        sid_value = atoi(sid->valuestring);
    }
    else {
        cJSON_Delete(root);
        return false;
    }

    strlcpy(out->status, status->valuestring, sizeof(out->status));

    out->sid = sid_value;

    cJSON_Delete(root);
    return true;
}