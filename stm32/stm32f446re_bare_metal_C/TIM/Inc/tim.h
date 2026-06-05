#ifndef TIM_H_
#define TIM_H_

#include "stm32f4xx.h"

#define SR_UIF		(1U<<0) // UIF: Update interrupt flag of timer

void tim2_1hz_init(void);

#endif
