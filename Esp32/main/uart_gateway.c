#include "uart_gateway.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "uart_protocol.h"
#include "esp_log.h"
#include "esp_err.h"

#define UART_PORT       UART_NUM_2
#define UART_RX_BUF_SIZE 1024
#define UART_TX_BUF_SIZE 1024
#define UART_TXD_PIN    GPIO_NUM_16
#define UART_RXD_PIN    GPIO_NUM_17

#define STX 0x02
#define ETX 0x03
#define MAX_PAYLOAD_LEN 255

static const char *TAG = "uart_gateway";

static uint8_t payload_checksum(const uint8_t *payload, uint8_t len){
    uint8_t chk = 0;
    for (uint8_t i = 0; i < len; i++) {
        chk ^= payload[i];
    }
    return chk;
}

static esp_err_t uart_send_payload(const char *payload){
    size_t payload_len = strlen(payload);
    if (payload_len > MAX_PAYLOAD_LEN) {
        ESP_LOGE(TAG, "payload too long: %u", (unsigned) payload_len);
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t frame[1 + 1 + MAX_PAYLOAD_LEN + 1 + 1];
    frame[0] = STX;
    frame[1] = (uint8_t) payload_len;
    memcpy(&frame[2], payload, payload_len);
    frame[2 + payload_len] = payload_checksum((const uint8_t *) payload, (uint8_t) payload_len);
    frame[3 + payload_len] = ETX;

    int written = uart_write_bytes(UART_PORT, frame, payload_len + 4);
    ESP_LOGI(TAG, "UART TX: %s", payload);
    return written == (int) payload_len + 4 ? ESP_OK : ESP_FAIL;
}

void uart_send_command(int sid, const char *command, const char *data){
    char payload[MAX_PAYLOAD_LEN + 1];
    snprintf(payload, sizeof(payload), "%d|%s|%s", sid, command, data ? data : "");
    uart_send_payload(payload);
}

void uart_send_nack(int sid, const char *reason){
    uart_send_command(sid, "NACK", reason);
}

static void init_uart(void){
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TXD_PIN, UART_RXD_PIN,
                                UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

static void rx_task(void *arg){
    uint8_t byte;
    uint8_t payload[MAX_PAYLOAD_LEN + 1];
    uint8_t len = 0;
    uint8_t pos = 0;
    uint8_t chk = 0;

    enum {
        WAIT_STX,
        WAIT_LEN,
        WAIT_PAYLOAD,
        WAIT_CHK,
        WAIT_ETX
    } state = WAIT_STX;

    while (1) {
        int n = uart_read_bytes(UART_PORT, &byte, 1, pdMS_TO_TICKS(1000));
        if (n <= 0) continue;

        switch (state) {
        case WAIT_STX:
            if (byte == STX) state = WAIT_LEN;
            break;

        case WAIT_LEN:
            len = byte;
            if (len == 0) {
                uart_send_nack(0, "BAD_LENGTH");
                state = WAIT_STX;
            } else {
                pos = 0;
                chk = 0;
                state = WAIT_PAYLOAD;
            }
            break;

        case WAIT_PAYLOAD:
            payload[pos++] = byte;
            chk ^= byte;
            if (pos >= len) state = WAIT_CHK;
            break;

        case WAIT_CHK:
            if (byte != chk) {
                uart_send_nack(0, "BAD_CHECKSUM");
                state = WAIT_STX;
            } else {
                state = WAIT_ETX;
            }
            break;

        case WAIT_ETX:
            if (byte != ETX) {
                uart_send_nack(0, "BAD_FORMAT");
                state = WAIT_STX;
                break;
            }

            payload[len] = '\0';
            ESP_LOGI(TAG, "UART RX payload: %s", (char *) payload);

            access_message_t msg = {0};
            char payload_copy[MAX_PAYLOAD_LEN + 1];
            strlcpy(payload_copy, (char *) payload, sizeof(payload_copy));

            if (!parse_payload(payload_copy, &msg)) {
                uart_send_nack(0, "BAD_FORMAT");
            } else {
                handle_access_message(&msg);
            }
            state = WAIT_STX;
            break;
        }
    }
}

void uart_gateway_init(void) {
    init_uart();
}

void uart_gateway_start(void) {
    xTaskCreate(rx_task, "uart_rx_task", 4096, NULL, 10, NULL);
}
