#include "uart.h"
#include "systick.h"

int main(void)
{
	uart_init(115200);
	while(1)
	{
		uart_write_string("UART Bare Metal On STM32!\n\r");
		uart_write_int(14);
		uart_write_string("\n\r");
		uart_write_int(-14);
		uart_write_string("\n\r");
		systick_delay_ms(1000);		
	}
}
