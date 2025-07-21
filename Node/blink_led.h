#ifndef BLINK_LED_H
#define BLINK_LED_H

#include <stdint.h>

#define LED_RED_PORT 0
#define LED_RED_PIN 26
#define LED_RED_INDEX 0

void init_blink_led();
void blink_led();
#endif