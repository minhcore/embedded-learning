#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32f4xx.h"
#include "stdint.h"

void systick_delay_ms(uint32_t expect_tick_ms);

#endif
