#ifndef UART_H
#define UART_H

#include <stdint.h>

#include "nrf52840.h"

#define UART_TX_PORT 0
#define UART_TX_PIN 4
#define UART_RX_PORT 0
#define UART_RX_PIN 5

#define UART_MAX_BUFFER_SIZE 1024

extern uint8_t uart_buf[];
extern uint16_t uart_start, uart_end;
extern uint32_t events_rxdrdy_count;


void uart_init();
void uart_send(uint8_t data);

#endif