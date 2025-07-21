#ifndef RF_DETECT
#define RF_DETECT

#include <stdint.h>

#define RUNNING 0
#define IDLE 1
#define WALKING 2
#define UNKNOW 3

#define NUM_SAMPLES 10
#define NUM_GESTURES 3
uint8_t rf_detect(float aX, float aY, float aZ);
void rf_detect_many_times();
extern uint8_t result_ai_status;


#endif