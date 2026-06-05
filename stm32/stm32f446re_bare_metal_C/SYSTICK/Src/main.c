#include "systick.h"
#include "gpio.h"

int main(void)
{
	led_init();
	led_off();
	while(1)
	{
		led_toggle();
		systick_delay_ms(1000);
	}
}
