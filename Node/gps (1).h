#ifndef GPS_H
#define GPS_H

#include <stdint.h>

/* D8 */
#define GPS_ON_OFF_PORT 0
#define GPS_ON_OFF_PIN 3

extern int gps_begin_index, gps_end_index;
extern float lat, lon;
extern float lat_before_turn_off, lon_before_turn_off;
extern float lat_last_sent, lon_last_sent;
extern uint8_t is_gps_valid;

void handle_nmea_sentence(uint8_t *nmea_buffer, int begin, int end);
void turn_off_gps();
void turn_on_gps();
void gps_init();
void fsm_gps();
void update_gps_timer();
void set_lora_frame(float lat_data, float lon_data, uint8_t ai_status);



// enum fsm_gps_state_t {
//   FSM_GPS_INIT, 
//   FSM_GPS_WALKING_TURN_ON, FSM_GPS_WALKING_TURN_OFF, 
//   FSM_GPS_IDLE_TURN_ON, FSM_GPS_IDLE_TURN_OFF, 
//   FSM_GPS_RUNNING_TURN_ON
// };

enum fsm_gps_state_t {
  FSM_GPS_INIT, 
  FSM_GPS_TURN_ON, FSM_GPS_TURN_OFF
};
#endif