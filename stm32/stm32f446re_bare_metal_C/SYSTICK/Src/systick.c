#include "systick.h"

#define CTRL_ENABLE			(1U<<0)
#define INT_ENABLE			(1U<<1)
#define INTERNAL_CLOCK		(1U<<2)
#define CLKS_SRC_MHZ		16
#define COUNT_FLAG			(1U<<16)

void systick_delay_ms(uint32_t expect_tick_ms)
{
	// Load number of clock cycles per milisecond
	uint32_t reload_value = (CLKS_SRC_MHZ * 1000) - 1;
	// Load reload value register
	SysTick->LOAD = reload_value;

	// Clear SysTick current value register
	SysTick->VAL = 0;

	// Select internal clock source
	SysTick->CTRL = INTERNAL_CLOCK;

	// Enable SysTick
	SysTick->CTRL |= CTRL_ENABLE;

	for(int i = 0; i < expect_tick_ms; i++)
	{
		while((SysTick->CTRL & COUNT_FLAG) == 0) {}
	}

	// Disable SysTick
	SysTick->CTRL &=~ CTRL_ENABLE;
}
