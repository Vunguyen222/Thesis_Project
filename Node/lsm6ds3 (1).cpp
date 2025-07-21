#include "lsm6ds3.h"


float accelX = 0.0, accelY = 0.0, accelZ = 0.0;

//Create a instance of class LSM6DS3
static LSM6DS3 myIMU(I2C_MODE, 0x6A);  //I2C device address 0x6A

void lsm6ds3_init() {
  myIMU.settings.gyroEnabled = 0;
  myIMU.settings.tempEnabled = 0;
  myIMU.settings.accelBandWidth = 50;
  myIMU.settings.accelSampleRate = 13;
  myIMU.settings.accelFifoEnabled = 0;

  myIMU.begin();

  // Bật chế độ low-power mode
  uint8_t ctrl6;
  myIMU.readRegister(&ctrl6, 0x15);
  ctrl6 |= (1 << 4);  // Set bit XL_HM_MODE
  myIMU.writeRegister(0x15, ctrl6); // 0x15 = CTRL6_C
}


void read_lsm6ds3() {

  accelX = myIMU.readFloatAccelX() * 9.80665;
  accelY = myIMU.readFloatAccelY() * 9.80665;
  accelZ = myIMU.readFloatAccelZ() * 9.80665;
}