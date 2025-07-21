#include <stdint.h>

#include "fsm_send.h"
#include "gps.h"
#include "rf_detect.h"
#include "lora.h"

#define SEND_TIMEOUT 20000 // 20s
#define MOVE_THRSHOLD 0.000045

bool is_moved(float lat1, float lon1, float lat2, float lon2) {
  float dlat = lat2 - lat1;
  dlat = dlat > 0 ? dlat : -dlat;

  float dlon = lon2 - lon1;
  dlon = dlon > 0 ? dlon : -dlon;

  return (dlat > MOVE_THRSHOLD) || (dlon > MOVE_THRSHOLD);
}


fsm_send_state_t fsm_send_state = FSM_SEND_INIT;
int send_timer = SEND_TIMEOUT;
void fsm_send_func() {
  send_timer -= 100;
  switch(fsm_send_state) {
    case FSM_SEND_INIT:
      fsm_send_state = FSM_SEND_INVALID_GPS;
    case FSM_SEND_INVALID_GPS:
      if(send_timer <= 0) {
        send_timer = SEND_TIMEOUT;
        if(lat_before_turn_off != 0.0) {
          set_lora_frame(lat_before_turn_off, lon_before_turn_off, result_ai_status);
        }else {
          set_lora_frame(0.0, 0.0, result_ai_status);
        }
        is_send = 1;
        break;
      }

      if(is_gps_valid) {
        fsm_send_state = FSM_SEND_VALID_GPS;
      }
      break;
    case FSM_SEND_VALID_GPS:
      if(send_timer <= 0) {
        send_timer = SEND_TIMEOUT;
        set_lora_frame(lat, lon, result_ai_status);
        is_send = 1;
        break;
      }

      if(is_moved(lat, lon, lat_last_sent, lon_last_sent)) {
        send_timer = SEND_TIMEOUT;
        set_lora_frame(lat, lon, result_ai_status);
        is_send = 1;
        break;
      }

      if(is_gps_valid == 0) {
        fsm_send_state = FSM_SEND_INVALID_GPS;
      }
      break;
    default: break;
  }
}