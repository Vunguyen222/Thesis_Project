#ifndef ACCEL_H
#define ACCEL_H

#include <math.h>

#include "statistics.h"


class Accel {
private:
  Statistics statistics_x, statistics_y, statistics_z, statistics_amplitude;

public:
  int size;
  int start_idx;
public:
  float features[16];
  void add_val(float aX, float aY, float aZ);
  Accel();
};

Accel::Accel(){
  this->size = 0;
  this->start_idx = 0;
}

void Accel::add_val(float aX, float aY, float aZ) {
  statistics_x.add_val(aX);
  statistics_y.add_val(aY);
  statistics_z.add_val(aZ);
  statistics_amplitude.add_val(sqrt(aX*aX + aY*aY + aZ*aZ));

  this->size = statistics_x.size;
  this->start_idx = statistics_x.start_idx;

  if(this->size == NUM_SAMPLES) {
    features[0] = statistics_x.get_mean();
    features[1] = statistics_y.get_mean();
    features[2] = statistics_z.get_mean();
    features[3] = statistics_amplitude.get_mean();

    features[4] = statistics_x.get_std();
    features[5] = statistics_y.get_std();
    features[6] = statistics_z.get_std();
    features[7] = statistics_amplitude.get_std();
    
    features[8] = statistics_x.get_max();
    features[9] = statistics_y.get_max();
    features[10] = statistics_z.get_max();
    features[11] = statistics_amplitude.get_max();

    features[12] = statistics_x.get_min();
    features[13] = statistics_y.get_min();
    features[14] = statistics_z.get_min();
    features[15] = statistics_amplitude.get_min();
  }
}

#endif