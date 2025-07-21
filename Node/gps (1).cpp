#include <stdlib.h>

#include <Arduino.h>
#include "nrf52840.h"
#include "gps.h"
#include "uart.h"
#include "rf_detect.h"
#include "lora.h"
#include "Adafruit_TinyUSB.h"

#define GPS_TURN_OFF_TIMER 20
int gps_begin_index = -1, gps_end_index = 0;
float lat = 0.0, lon = 0.0;
float lat_before_turn_off = 0.0, lon_before_turn_off = 0.0;
float lat_last_sent = 0.0, lon_last_sent = 0.0;
uint8_t is_gps_valid = 0;

void update_gps_before_turn_off() {
  lat_before_turn_off = lat;
  lon_before_turn_off = lon;
}

bool is_number(uint8_t data)
{
    return data <= '9' && data >= '0';
}

float nmea_to_decimal(char *nmea_coord, int length) {
    if (length == 0) return 0.0;

    nmea_coord[length] = '\0';

    /* Find '.' index */
    int dot_index = 0;
    for(; nmea_coord[dot_index] != '.' && dot_index < length; dot_index ++);
    
    int degrees = 0;
    for(int i = 0; i < dot_index - 2; i++) {
        degrees *= 10;
        degrees += (nmea_coord[i] - '0');
    }

    float minutes = atof(nmea_coord + dot_index - 2);
    float decimal_degrees = (float)degrees + (minutes / 60.0);
    return decimal_degrees;
}

bool is_gngll(uint8_t *nmea_buffer, int begin) {
  char gngll_buf[] = "GNGLL";
  for(int i = 0; i < 5; i++) {
    if(nmea_buffer[(begin + i) % UART_MAX_BUFFER_SIZE] != gngll_buf[i]) return false;
  }

  return true;
}

void handle_nmea_sentence(uint8_t *nmea_buffer, int begin, int end)
{
    if(is_gngll(nmea_buffer, begin) == false) {
      return;
    }

    is_gps_valid = 0;
    int calculated_checksum = 0;
    int n_token = 0; // ','
    char str_lat[20], str_lon[20];
    int index_str_lat = 0, index_str_lon = 0;

    
    for(; nmea_buffer[begin] != '*' && begin != end; begin = (begin + 1) % UART_MAX_BUFFER_SIZE) {
        calculated_checksum ^= nmea_buffer[begin];
        if(nmea_buffer[begin] == ',') {
            n_token ++;
            continue;
        }
        if(n_token == 1) {
            str_lat[index_str_lat ++] = nmea_buffer[begin];
        }else if(n_token == 3) {
            str_lon[index_str_lon ++] = nmea_buffer[begin];
        }
    }

    uint8_t left_byte = is_number(nmea_buffer[(begin + 1) % UART_MAX_BUFFER_SIZE]) ? 
                        nmea_buffer[(begin + 1) % UART_MAX_BUFFER_SIZE] - '0' : 
                        nmea_buffer[(begin + 1) % UART_MAX_BUFFER_SIZE] - 'A' + 10;
    uint8_t right_byte = is_number(nmea_buffer[(begin + 2) % UART_MAX_BUFFER_SIZE]) ? 
                        nmea_buffer[(begin + 2) % UART_MAX_BUFFER_SIZE] - '0' : 
                        nmea_buffer[(begin + 2) % UART_MAX_BUFFER_SIZE] - 'A' + 10;
    uint8_t expected_checksum = (left_byte << 4) | right_byte;

    
    bool result_checksum = calculated_checksum == expected_checksum;

    if(result_checksum == false) {
        lat = 0.0;
        lon = 0.0;
        return;
    }
    lat = nmea_to_decimal(str_lat, index_str_lat);
    lon = nmea_to_decimal(str_lon, index_str_lon);
    is_gps_valid = index_str_lat > 0;
    return;
}

void gps_init() {
  uart_init();
  NRF_GPIOTE->CONFIG[2] = 0x3 | (GPS_ON_OFF_PIN << 8) | (GPS_ON_OFF_PORT << 13);
  turn_on_gps();
}

void turn_off_gps() {
  // pull down
  NRF_GPIOTE->TASKS_CLR[2] = 1;
}
void turn_on_gps() {
  // pull up
  NRF_GPIOTE->TASKS_SET[2] = 1;

  char config_output[] = "$PCAS03,0,1,0,0,0,0,0,0,0,0,0,0,0*1F\r\n";
  for(int i = 0; config_output[i] != '\0'; i++) {
    uart_send(config_output[i]);
  }

}

static uint8_t cal_checksum(uint8_t *data, uint16_t len) {
    uint8_t result = 0x00;
    for(uint16_t i = 0; i < len; i++) {
        result ^= data[i];
    }
    return result;
}



void set_lora_frame(float lat_data, float lon_data, uint8_t ai_status) {
  
  if(lat_data != 0.0) {
    // Serial.println("set gps frame");
    lat_last_sent = lat_data;
    lon_last_sent = lon_data;
    
    lora_frame.ai_gps_frame.is_gps = 1;
    lora_frame.ai_gps_frame.start_byte = '!';
    lora_frame.ai_gps_frame.addr = 1;
    lora_frame.ai_gps_frame.lat = lat_data;
    lora_frame.ai_gps_frame.lon = lon_data;
    lora_frame.ai_gps_frame.ai_status = ai_status;
    lora_frame.ai_gps_frame.stop_byte = '#';
    lora_frame.ai_gps_frame.checksum = cal_checksum((uint8_t*)&lora_frame.ai_gps_frame + 1, sizeof(ai_gps_frame_t) - 2);
  }else {
    // Serial.println("set ai frame");
    lora_frame.ai_frame.is_gps = 0;
    lora_frame.ai_frame.start_byte = '!';
    lora_frame.ai_frame.addr = 1;
    lora_frame.ai_frame.ai_status = ai_status;
    lora_frame.ai_frame.stop_byte = '#';
    lora_frame.ai_frame.checksum = cal_checksum((uint8_t*)&lora_frame.ai_frame + 1, sizeof(ai_frame_t) - 2);
  }
}




#define GPS_TURN_ON_TIMEOUT 20
#define GPS_TURN_OFF_TIMEOUT 20

int gps_turn_on_timer = GPS_TURN_ON_TIMEOUT;
int gps_turn_off_timer = GPS_TURN_OFF_TIMEOUT;
void update_gps_timer() {
  gps_turn_off_timer--;
  gps_turn_on_timer--;
}

fsm_gps_state_t fsm_gps_state = FSM_GPS_INIT;
void fsm_gps() {
  update_gps_timer();
  switch(fsm_gps_state) {
    case FSM_GPS_INIT:
      fsm_gps_state = FSM_GPS_TURN_ON;
    case FSM_GPS_TURN_ON:
      // turn_on_gps();
      if(result_ai_status == IDLE && (gps_turn_off_timer <= 0 || is_gps_valid)) {
        turn_off_gps();
        fsm_gps_state = FSM_GPS_TURN_OFF;
        gps_turn_on_timer = GPS_TURN_ON_TIMEOUT;
        if(is_gps_valid) {
          update_gps_before_turn_off();
        }
      }
      break;
    case FSM_GPS_TURN_OFF:
      // turn_off_gps();
      if(result_ai_status != IDLE || gps_turn_on_timer <= 0) {
        turn_on_gps();
        fsm_gps_state = FSM_GPS_TURN_ON;
        gps_turn_off_timer = GPS_TURN_OFF_TIMEOUT;
      }
      break;
    default: break;
  }
}

// void fsm_gps(uint8_t ai_status) {
//   if((ai_status == IDLE) && 
//      (fsm_gps_state != FSM_GPS_IDLE_TURN_ON && fsm_gps_state != FSM_GPS_IDLE_TURN_OFF)) {
//         fsm_gps_state = FSM_GPS_INIT;
//      }
//   else if((ai_status == WALKING) && 
//           (fsm_gps_state != FSM_GPS_WALKING_TURN_ON && fsm_gps_state != FSM_GPS_WALKING_TURN_OFF)) {
//             fsm_gps_state = FSM_GPS_INIT;
//           }
//   else if((ai_status == RUNNING) && 
//           (fsm_gps_state != FSM_GPS_RUNNING_TURN_ON)) {
//             fsm_gps_state = FSM_GPS_INIT;
//           }

  
//   switch(fsm_gps_state) {
//     case FSM_GPS_INIT:
//       if (ai_status == WALKING) {
//         fsm_gps_state = FSM_GPS_WALKING_TURN_ON;
//         /* Turn on GPS */
//         turn_on_gps();
//         /* Set GPS turn off timer */
//         if(gps_turn_off_timer < 0 || gps_turn_off_timer > GPS_TURN_OFF_TIMER) {
//           gps_turn_off_timer = GPS_TURN_OFF_TIMER; /* 60 seconds */
//         }
//         gps_turn_on_timer = -1;
//       } else if (ai_status == IDLE) {
//         fsm_gps_state = FSM_GPS_IDLE_TURN_ON;
//         /* Turn on GPS */
//         turn_on_gps();
//         /* Set GPS turn off timer */
//         if(gps_turn_off_timer < 0 || gps_turn_off_timer > GPS_TURN_OFF_TIMER) {
//           gps_turn_off_timer = GPS_TURN_OFF_TIMER; /* 60 seconds */
//         }
//         gps_turn_on_timer = -1;
//       } else if (ai_status == RUNNING) {
//         fsm_gps_state = FSM_GPS_RUNNING_TURN_ON;
//         /* Turn on GPS */
//         turn_on_gps();
//         gps_turn_off_timer = -1;
//         gps_turn_on_timer = -1;
//       }
//       break;

//     case FSM_GPS_WALKING_TURN_ON:
//       if(is_gps_valid) {
//         fsm_gps_state = FSM_GPS_WALKING_TURN_OFF;
//         /* Send GPS data */
//         set_lora_frame(is_gps_valid, ai_status);
//         /* Turn off GPS */
//         turn_off_gps();
//         /* Set GPS turn on timer */
//         gps_turn_on_timer = 10; /* Module GPS will turn on after 10 seconds */
//         gps_turn_off_timer = -1;
//         break;
//       }

//       if(gps_turn_off_timer == 0) {
//         fsm_gps_state = FSM_GPS_WALKING_TURN_OFF;
//         /* Send GPS data */
//         set_lora_frame(is_gps_valid, ai_status);
//         /* Turn off GPS */
//         turn_off_gps();
//         /* Set GPS turn on timer */
//         gps_turn_on_timer = 10; /* Module GPS will turn on after 10 seconds */
//         gps_turn_off_timer = -1;
//         break;
//       }
//       break;

//     case FSM_GPS_WALKING_TURN_OFF:
//       if(gps_turn_on_timer == 0) {
//         fsm_gps_state = FSM_GPS_WALKING_TURN_ON;
//         turn_on_gps();
//         /* Set GPS turn off timer */
//         gps_turn_off_timer = GPS_TURN_OFF_TIMER; /* Module GPS will turn off after GPS_TURN_OFF_TIMER seconds */
//         gps_turn_on_timer = -1;
//       }
//       break;

//     case FSM_GPS_IDLE_TURN_ON:
//       if(is_gps_valid) {
//         fsm_gps_state = FSM_GPS_IDLE_TURN_OFF;
//         /* Send GPS data */
//         set_lora_frame(is_gps_valid, ai_status);
//         /* Turn off GPS */ 
//         turn_off_gps();
//         /* Set GPS turn on timer */
//         gps_turn_on_timer = 20; /* Module GPS will turn on after 20 seconds */
//         gps_turn_off_timer = -1;
//         break;
//       }

//       if(gps_turn_off_timer == 0) {
//         fsm_gps_state = FSM_GPS_IDLE_TURN_OFF;
//         /* Send AI data */
//         set_lora_frame(is_gps_valid, ai_status);

//         /* Turn off GPS */ 
//         turn_off_gps();
//         /* Set GPS turn off timer */
//         gps_turn_on_timer = 20; /* Module GPS will turn on after 60 seconds */
//         gps_turn_off_timer = -1;
//         break;
//       }
//       break;

//     case FSM_GPS_IDLE_TURN_OFF:
//       if(gps_turn_on_timer == 0) {
//         fsm_gps_state = FSM_GPS_IDLE_TURN_ON;
//         turn_on_gps();
//         /* Set GPS turn on timer */
//         gps_turn_off_timer = GPS_TURN_OFF_TIMER; /* Module GPS will turn off after GPS_TURN_OFF_TIMER seconds */
//         gps_turn_on_timer = -1;
//         break;
//       }
//       break;

//     case FSM_GPS_RUNNING_TURN_ON:
//       if(is_gps_valid) {
//         /* Send GPS data */
//         set_lora_frame(is_gps_valid, ai_status);
//         break;
//       }else {
//         if(send_running_status_timer <= 0) {
//           send_running_status_timer = 10; /* Module LORA will send running status after 60 seconds */
//           set_lora_frame(is_gps_valid, ai_status);
//         }else {
//           send_running_status_timer --;
//         }
//       }
//       break;

//     defaut: break;
//   }
// }

