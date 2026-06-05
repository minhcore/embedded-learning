#include "tim.h"
#include "gpio.h"

int main(void)
{
	tim2_1hz_init();
	led_init();
	led_off();
	while(1)
	{
		led_toggle();
		while((TIM2->SR & SR_UIF) == 0) {}
		// Clear UIF
		TIM2->SR &=~ SR_UIF;
	}
}
