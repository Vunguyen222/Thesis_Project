#include "rtc.h"
#include "nrf52840.h"

void init_rtc() {
  // Bật LFCLK
  NRF_CLOCK->LFCLKSRC = 1;
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);

  // Cấu hình RTC1 cho tick 100ms
  NRF_RTC0->PRESCALER = 255;       // Tick mỗi 7.8125ms
  NRF_RTC0->CC[0] = 13;            // 13 ticks ≈ 101.56ms

  NRF_RTC0->EVTENSET = 1 << 16;
  NRF_RTC0->INTENSET = 1 << 16;

  NRF_RTC0->TASKS_START = 1;

  NVIC_SetPriority(RTC0_IRQn, 15);  // Lowes priority
  NVIC_ClearPendingIRQ(RTC0_IRQn);
  NVIC_EnableIRQ(RTC0_IRQn);
}
