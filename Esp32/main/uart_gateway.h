#pragma once

void uart_gateway_init(void);
void uart_gateway_start(void);
void uart_send_command(int sid, const char *cmd, const char *data);
void uart_send_nack(int sid, const char *reason);
