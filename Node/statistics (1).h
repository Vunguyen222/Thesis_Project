#ifndef STATISTICS_H
#define STATISTICS_H
#include <stdint.h>


#define NUM_SAMPLES 10

class Statistics {
private:
  float nums[NUM_SAMPLES];
  float min;
  float max;
  float variance;
  float k, ex, ex2;
  float mean;
public:
  int size;
  int start_idx;


public:
  void add_val(float new_val);
  float get_max();
  float get_min();
  float get_std();
  float get_mean();

private:
  /* Call cal_std before call cal_mean */
  void cal_variance(float new_val, float rm_val);

  /* Call cal_mean before add new_val to nums */
  void cal_mean(float new_val, float rm_val);

  void cal_min(float new_val, float rm_val);
  void cal_max(float new_val, float rm_val);
  
public:
  Statistics();
};

float Statistics::get_max(){
  return this->max;
}

float Statistics::get_min() {
  return this->min;
}

float Statistics::get_std() {
  return sqrt(this->variance);
}

float Statistics::get_mean() {
  return this->mean;
}

void Statistics::add_val(float new_val) {
    float rm_val = 0;
    bool is_full = (size == NUM_SAMPLES);
    int next_idx;
    if (is_full) {
      rm_val = nums[start_idx];  // Lấy giá trị cũ bị loại bỏ
      next_idx = start_idx;
      start_idx = (start_idx + 1) % NUM_SAMPLES; // Cập nhật vị trí FIFO
    }else {
      next_idx = (start_idx + size) % NUM_SAMPLES;
    }
    nums[next_idx] = new_val;

    cal_variance(new_val, rm_val);
    cal_mean(new_val, rm_val);
    
    if (!is_full) {
      size++;
    }

    // Cập nhật các giá trị thống kê
    cal_min(new_val, rm_val);
    cal_max(new_val, rm_val);
}


void Statistics::cal_variance(float new_val, float rm_val) {
  if(this->size == 0) {
    this->k = new_val;
  }

  /* add_variable */
  int n = this->size + 1;
  ex += new_val - k;
  ex2 += (new_val-k)*(new_val-k);

  if(n == NUM_SAMPLES + 1) {
    /* remove_variable */
    n --;
    ex -= rm_val - k;
    ex2 -= (rm_val - k)*(rm_val - k);
  }

  if(n == NUM_SAMPLES) {
    variance = (ex2 - ex*ex/n) / (n-1);
  }

}


void Statistics::cal_mean(float new_val, float rm_val) {
  float sum = this->mean * this->size;
  sum = sum + new_val - rm_val;
  this->mean = this->size == NUM_SAMPLES ? sum / this->size : sum / (this->size + 1);
}

void Statistics::cal_max(float new_val, float rm_val) {
  if(new_val >= this->max) {
    this->max = new_val;
  }else {
    if(rm_val == this->max) {
      /* Find max */
      this->max = nums[this->start_idx];
      for(int i = 1; i < this->size; i++) {
        int next_idx = (this->start_idx + i) % NUM_SAMPLES;
        if(nums[next_idx] > this->max) {
          this->max = nums[next_idx];
        }
      }
    }
  }
}

void Statistics::cal_min(float new_val, float rm_val) {
  if(new_val <= this->min) {
    this->min = new_val;
  }else {
    if(rm_val == this->min) {
      /* Find min */
      this->min = nums[start_idx];
      for(int i = 1; i < this->size; i++) {
        int next_idx = (this->start_idx + i) % NUM_SAMPLES;
        if(nums[next_idx] < this->min) {
          this->min = nums[next_idx];
        }
      }
    }
  }

}


Statistics::Statistics() {
  size = 0;
  start_idx = 0;
  mean = 0.0;
  variance = 0.0;
  min = 999.0;
  max = -999.0;
  k = 0.0;
  ex = 0.0;
  ex2 = 0.0;
}


#endif