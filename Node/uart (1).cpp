#include <Arduino.h>
#include "uart.h"

/* Receive buffer */
uint8_t uart_buf[UART_MAX_BUFFER_SIZE];
uint16_t uart_start = 0, uart_end = 0;


void 
uart_init() {
  /* Config TX */
  NRF_UART0->PSEL.TXD = (UART_TX_PIN) | (UART_TX_PORT << 5);
  /* Config RX */
  NRF_UART0->PSEL.RXD = (UART_RX_PIN) | (UART_RX_PORT << 5);

  /* Config baudrate 9600 */
  NRF_UART0->BAUDRATE = 0x00275000;

  /**
  * Enable interrupt:
  *   - RXDRDY: Data received in RXD (2)
  *   - TXDRDY: Data sent from TXD (7)
  */
  NRF_UART0->INTENSET = (1 << 2);

  /* Enable UARTE */
  NRF_UART0->ENABLE = 4;
  
  /* Trigger RX task */
  NRF_UART0->TASKS_STARTRX = 1;

  /* Enable ISR */
  NVIC_SetPriority(UARTE0_UART0_IRQn, 15);
  NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
  NVIC_EnableIRQ(UARTE0_UART0_IRQn);
}

void uart_send(uint8_t data) {
    NRF_UART0->EVENTS_TXDRDY = 0;           // Clear TX ready event
    NRF_UART0->TXD = data;                  // Load data into transmit register
    NRF_UART0->TASKS_STARTTX = 1;           // Start transmission

    while (NRF_UART0->EVENTS_TXDRDY == 0);  // Wait for transmission to complete
    NRF_UART0->EVENTS_TXDRDY = 0;           // Clear again for next transmission
}

int uart_interrupt_cnt = 0;
extern "C" {
  void UARTE0_UART0_IRQHandler() {
    uart_interrupt_cnt ++;
    if (NRF_UART0->EVENTS_RXDRDY) {
      NRF_UART0->EVENTS_RXDRDY = 0;
      NRF_UART0->TASKS_STARTRX = 1;
      /* Copy data to buffer */
      uart_buf[uart_end] = NRF_UART0->RXD;
      uart_end = (uart_end + 1) % UART_MAX_BUFFER_SIZE;
    }
  }
}


