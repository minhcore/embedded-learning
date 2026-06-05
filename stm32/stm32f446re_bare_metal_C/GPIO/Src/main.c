#include "gpio.h"

int main(void)
{
	button_init();
	led_init();
	led_off();
	while(1)
	{
		if (get_btn_state())
		{
			led_on();
		}
		else
		{
			led_off();
		}
	}
}
