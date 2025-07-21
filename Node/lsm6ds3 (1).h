#ifndef LSM6DS3_H
#define LSM6DS3_h

#include <LSM6DS3.h>
#include <Wire.h>

#define IMU_INT1_PORT 0
#define IMU_INT1_PIN 11


extern float accelX, accelY, accelZ;

void lsm6ds3_init();
void read_lsm6ds3();
#endif
