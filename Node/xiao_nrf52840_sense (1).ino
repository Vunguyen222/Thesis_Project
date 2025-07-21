#include "nrf52840.h"
#include "rtc.h"
#include "fsm_send.h"


#define LORA
// #define DEBUG
#define LSM6DS3_SENSOR
#define GPS
#define BLINK_LED

#ifdef LSM6DS3_SENSOR
  #include "lsm6ds3.h"
  #include "rf_detect.h"
#endif

#ifdef BLINK_LED
  #include "blink_led.h"
#endif

#ifdef GPS
#include "gps.h"
#include "uart.h"
#endif

#ifdef DEBUG
  #include "Adafruit_TinyUSB.h"
#endif

#ifdef LORA
  #include "lora.h"
#endif


#ifdef GPS
  extern fsm_gps_state_t fsm_gps_state;
  extern int gps_turn_on_timer;
  extern int gps_turn_off_timer;
#endif

#define DETECT_AI_TIMER_DURATION 10
#define FSM_GPS_DURATION 10

extern uint8_t result_ai_status;

int detect_ai_timer = DETECT_AI_TIMER_DURATION;

extern "C" {
  void RTC0_IRQHandler() {
    
    if (NRF_RTC0->EVENTS_COMPARE[0]) {
      NRF_RTC0->EVENTS_COMPARE[0] = 0;
      NRF_RTC0->CC[0] += 13;  

      /* Detect AI */
      detect_ai_timer --;
      if(detect_ai_timer <= 0) {
        detect_ai_timer = DETECT_AI_TIMER_DURATION;
        
        read_lsm6ds3();
        rf_detect_many_times();

        /* FSM GPS */
        fsm_gps();
      }

      /* If interrupt does not run detect AI function */
      if(detect_ai_timer != DETECT_AI_TIMER_DURATION) {
        /* Handle GPS data */
        while(uart_start != uart_end) {
          if(uart_buf[uart_start] == '$') {
            gps_begin_index = uart_start;
          } else if(uart_buf[uart_start] == '\r') {
            gps_end_index = uart_end;

            handle_nmea_sentence(uart_buf, gps_begin_index + 1, gps_end_index);
            gps_begin_index = -1;
            gps_end_index = -1;
          }
          uart_start = (uart_start + 1) % UART_MAX_BUFFER_SIZE;
        }
      }

      fsm_lora();
      fsm_send_func();
    }
  }
}

void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
    while (!Serial)
      ;
    Serial.println("Init Serial success");
  #endif

  #ifdef GPS
    gps_init();
    #ifdef DEBUG
      Serial.println("Init GPS success");
    #endif
  #endif

  #ifdef BLINK_LED
    init_blink_led();
  #endif

  #ifdef LSM6DS3_SENSOR
    lsm6ds3_init();
    #ifdef DEBUG
      Serial.println("Init LSM6DS3 success");
    #endif
  #endif

  init_rtc();

  lora_init();
  turn_off_lora();
}

char send_lora_buf[] = "hello";


int blink_led_timer = 7;
uint8_t blink_led_status = 0;

int set_gps_sending_timer = 20;
uint8_t gps_sending_status = 1;


void loop() {
  #ifdef BLINK_LED
    blink_led();
  #endif

  #ifdef DEBUG
    // Serial.print("rtc0 cnt:");
    // Serial.println(rtc0_interrupt_cnt);
    #ifdef LSM6DS3_SENSOR
      // Serial.print("ai result:");
      // Serial.println(result_ai_status);
    #endif
    #ifdef GPS
      // Serial.print(fsm_gps_state);
      // Serial.print(",");
      // Serial.print(gps_turn_on_timer);
      // Serial.print(",");
      // Serial.println(gps_turn_off_timer);
    #endif
  #endif

  // write_lora((uint8_t*)send_lora_buf, 5);
  delay(1000);
}

