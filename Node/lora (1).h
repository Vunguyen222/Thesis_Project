#ifndef LORA_H
#define LORA_H

struct __attribute__((packed)) ai_gps_frame_t {
    uint8_t is_gps;
    uint8_t start_byte;
    uint16_t addr;
    float lat, lon;
    uint8_t ai_status;
    uint8_t stop_byte;
    uint8_t checksum;
};

struct __attribute__((packed)) ai_frame_t {
  uint8_t is_gps;
  int8_t start_byte;
  uint16_t addr;
  uint8_t ai_status;
  uint8_t stop_byte;
  uint8_t checksum;
};

union lora_frame_t {
  ai_gps_frame_t ai_gps_frame;
  ai_frame_t ai_frame;
};

extern lora_frame_t lora_frame;

extern uint8_t is_send;

void write_lora(uint8_t *buf, uint8_t length);
void lora_init();
void turn_on_lora();
void turn_off_lora();
void fsm_lora();
#endif