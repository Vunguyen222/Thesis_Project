#include <nrf52840.h>
#include "lora.h"
#include "Adafruit_TinyUSB.h"

#define LORA_TX_PORT 1
#define LORA_TX_PIN  11
#define LORA_RX_PORT 1
#define LORA_RX_PIN 12

#define LORA_ON_OFF_PORT 0
#define LORA_ON_OFF_PIN 2

#define LORA_MAX_RX_BUF 1024


enum fsm_lora_state_t {
  FSM_LORA_INIT,
  FSM_LORA_TURN_OFF, FSM_LORA_TURN_ON,
  FSM_LORA_SEND, FSM_LORA_WAITING_ACK
};

static fsm_lora_state_t fsm_lora_state = FSM_LORA_INIT;
static int fsm_lora_count = -1;
lora_frame_t lora_frame;
uint8_t is_send = 0;
uint8_t timeout_error_count = 2;
uint8_t is_ack;

uint8_t lora_rx_buf[LORA_MAX_RX_BUF];
uint16_t lora_rx_buf_offset = 0;
uint16_t lora_begin = 0;
uint16_t lora_len = 0;

void write_lora(uint8_t *buf, uint8_t length) {
  /* Data pointer */
  NRF_UARTE1->TXD.PTR = (uint32_t)(uintptr_t)buf;
  /* Maximum number of bytes in transmit buffer */
  NRF_UARTE1->TXD.MAXCNT = length;

  NRF_UARTE1->TASKS_STARTTX = 1;
}


void lora_init() {
  /* Select TX and RX pins */
  NRF_UARTE1->PSEL.TXD = (LORA_TX_PORT << 5) | LORA_TX_PIN;
  NRF_UARTE1->PSEL.RXD = (LORA_RX_PORT << 5) | LORA_RX_PIN;

  /* Config baudrate to 9600 */
  NRF_UARTE1->BAUDRATE = 0x00275000;

  /* Enable UARTE1 */
  NRF_UARTE1->ENABLE = 8;

  /* Config RX buffer */
  NRF_UARTE1->RXD.PTR = (uint32_t)(uintptr_t)lora_rx_buf;
  NRF_UARTE1->RXD.MAXCNT = LORA_MAX_RX_BUF / 2;
}

void turn_on_lora() {
  
  /* Clear and enable interrupts */
  NVIC_ClearPendingIRQ(UARTE1_IRQn);
  NVIC_EnableIRQ(UARTE1_IRQn);

  NRF_UARTE1->EVENTS_ENDRX = 0;
  NRF_UARTE1->EVENTS_RXSTARTED = 0;

  NRF_UARTE1->INTENSET = (1 << 4) | (1 << 2); // ENDRX and RXDRDY
  NRF_UARTE1->TASKS_STARTRX = 1;

  /* Turn on control lora pin */
  NRF_GPIOTE->CONFIG[3] = 0x3 | (LORA_ON_OFF_PIN << 8) | (LORA_ON_OFF_PORT << 13);
  NRF_GPIOTE->TASKS_SET[3] = 1;
}

void turn_off_lora() {
  /* Turn off control lora pin */
  NRF_GPIOTE->CONFIG[3] = 0x3 | (LORA_ON_OFF_PIN << 8) | (LORA_ON_OFF_PORT << 13);
  NRF_GPIOTE->TASKS_CLR[3] = 1;
}



extern "C" {
  void UARTE1_IRQHandler() {
    if(NRF_UARTE1->EVENTS_ENDRX) {
      NRF_UARTE1->EVENTS_ENDRX = 0;
      
      /* Update buffer */
      if(lora_rx_buf_offset == 0) {
        NRF_UARTE1->RXD.PTR = (uint32_t)(uintptr_t)(lora_rx_buf + LORA_MAX_RX_BUF / 2);
        lora_rx_buf_offset = 1;
      }else {
        NRF_UARTE1->RXD.PTR = (uint32_t)(uintptr_t)lora_rx_buf;
        lora_rx_buf_offset = 0;
      }
      NRF_UARTE1->RXD.MAXCNT = LORA_MAX_RX_BUF / 2;
      NRF_UARTE1->TASKS_STARTRX = 1;
    }

    if(NRF_UARTE1->EVENTS_RXDRDY) {
      NRF_UARTE1->EVENTS_RXDRDY = 0;
      lora_len ++;
    }
  }
}

uint8_t read_lora() {
  uint8_t result = lora_rx_buf[lora_begin++];
  lora_len --;
  return result;
}

static uint8_t cal_checksum(uint8_t *data, uint16_t len) {
    uint8_t result = 0x00;
    
    for(uint16_t i = 0; i < len; i++) {
        result ^= data[i];
    }
    return result;
}

void handle_lora_msg() {
  if(lora_len < 5) {
    return;
  }

  uint8_t temp_buf[5];
  while(lora_len > 0) {
    if(read_lora() == '!') {
      temp_buf[0] = '!';
      break;
    }
  }

  if(lora_len >= 4) {
    for(int i = 0; i < 4; i++) {
      temp_buf[1 + i] = read_lora();
    }
  }

  
  if(temp_buf[3] != '#') {
    return;
  }

  /* Check sum */
  uint8_t expected_sum = cal_checksum(temp_buf, 4);
  if(temp_buf[4] != expected_sum) {
    return;
  }

  uint16_t temp_addr = (temp_buf[2] << 8) |  temp_buf[1];
  
  // Serial.println(temp_addr);
  if(temp_addr == 1) {
    is_ack = 1;
    // Serial.println("Receive ack");
  }
}



void fsm_lora() {
  switch(fsm_lora_state) {
    case FSM_LORA_INIT:
      fsm_lora_state = FSM_LORA_TURN_OFF;
    case FSM_LORA_TURN_OFF:
      if(is_send) {
        fsm_lora_state = FSM_LORA_TURN_ON;
        
        is_ack = 0;
        is_send = 0;
        fsm_lora_count = 2;
        turn_on_lora();
        // Serial.println("turn on lora");
      }
      break;
    case FSM_LORA_TURN_ON:
      if(fsm_lora_count > 0) {
        fsm_lora_count --;
        if(fsm_lora_count <= 0) {
          fsm_lora_state = FSM_LORA_SEND;
          timeout_error_count = 2;
          /* send data */
          if(lora_frame.ai_gps_frame.is_gps) {
            write_lora((uint8_t*)&lora_frame.ai_gps_frame + 1, sizeof(ai_gps_frame_t) - 1);
          }else {
            write_lora((uint8_t*)&lora_frame.ai_frame + 1, sizeof(ai_frame_t) - 1);
          }
          // Serial.println("send lora");
        }
      }
      break;
    case FSM_LORA_SEND:
      fsm_lora_state = FSM_LORA_WAITING_ACK;
      fsm_lora_count = 20;
      break;
    case FSM_LORA_WAITING_ACK:
      handle_lora_msg();

      if(is_ack) {
        fsm_lora_state = FSM_LORA_TURN_OFF;
        turn_off_lora();
        // Serial.println("turn off lora");
        return;
      }
      if(fsm_lora_count > 0) {
        fsm_lora_count --;
        if(fsm_lora_count <= 0) {
          if(timeout_error_count > 0) {
            /* Resend */
            timeout_error_count --;
            fsm_lora_state = FSM_LORA_SEND;
            /* send data */
            if(lora_frame.ai_gps_frame.is_gps) {
              write_lora((uint8_t*)&lora_frame.ai_gps_frame + 1, sizeof(ai_gps_frame_t) - 1);
            }else {
              write_lora((uint8_t*)&lora_frame.ai_frame + 1, sizeof(ai_frame_t) - 1);
            }
            // Serial.println("resend lora");
          }else {
            fsm_lora_state = FSM_LORA_TURN_OFF;
            turn_off_lora();
            // Serial.println("turn off lora");
            return;
          }
        }
      }
      break;
    default: break;
  }
}

