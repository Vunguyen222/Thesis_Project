#include "nrf52840.h"

#include "blink_led.h"

void init_blink_led() {
  /* Config LED_RED for OUTPUT */
  NRF_GPIOTE->CONFIG[LED_RED_INDEX] = 0x3 | (LED_RED_PIN << 8) | (LED_RED_PORT << 13);
}

static uint8_t led_status = 0;
void blink_led() {
  if(led_status == 1){
    led_status = 0;
    NRF_GPIOTE->TASKS_CLR[LED_RED_INDEX] = 0x1;
  }else {
    led_status = 1;
    NRF_GPIOTE->TASKS_SET[LED_RED_INDEX] = 0x1;
  }
}
