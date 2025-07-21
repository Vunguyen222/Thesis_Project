#ifndef FSM_SEND_H
#define FSM_SEND_H

enum fsm_send_state_t {
  FSM_SEND_INIT, 
  FSM_SEND_INVALID_GPS, FSM_SEND_VALID_GPS
};

void fsm_send_func();
#endif