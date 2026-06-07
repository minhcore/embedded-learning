#include "uart.h"

#define SR_TXE			(1U<<7)

static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate);
static uint16_t compute_uart_brr(uint32_t periph_clk, uint32_t baudrate);
static void int_to_string(char* buff, int32_t number); 

void uart_init(uint32_t baudrate)
{
	// Enable clock access to GPIOA
	RCC->AHB1ENR |= (1U<<0);
	
	// Using USART2 (PA2-TX, PA3-RX)
	GPIOA->MODER |= 	(1U<<5);
	GPIOA->MODER &=~ 	(1U<<4);
	//GPIOA->MODER |=		(1U<<7);
	//GPIOA->MODER &=~	(1U<<6);

	// AF7 for PA2 and PA3
	GPIOA->AFR[0] |=	(1U<<8) | (1U<<9) | (1U<<10);
	GPIOA->AFR[0] &=~	(1U<<11);
	//GPIOA->AFR[0] |= 	(1U<<12) | (1U<<13) | (1U<<14);
	//GPIOA->AFR[0] &=~	(1U<<15);

	// Enable clock access to USART2
	RCC->APB1ENR |= (1U<<17);

	// Configure uart baudrate
	uart_set_baudrate(SYS_FREQ, baudrate);

	// Transmitter enable
	USART2->CR1 |= (1U<<3);

	// USART enable
	USART2->CR1 |= (1U<<13);
}

void uart_write_char(char c)
{
	// Wait for transmit data register is empty
	while(!(USART2->SR & SR_TXE)) {}

	// Write char c to transmit data register
	USART2->DR = c;
}

void uart_write_string(char* string)
{
	while ((*string) != '\0')
	{
		uart_write_char(*string);
		string++;
	}
}

void uart_write_int(int32_t number)
{
	char buf[40];
	int_to_string(buf, number);
	uart_write_string(buf);
}

static void int_to_string(char* buff, int32_t number)
{	
	char tmp_buff[40];
	uint8_t i = 0;
	uint8_t sign = 0;
	
	if (number < 0)
	{
		sign = 1;
		number = -number;
	}

	if (number == 0)
	{
		buff[0] = '0';
		buff[1] = '\0';
		return;
	}
	
	while (number > 0)
	{
		tmp_buff[i++] = '0' + (number % 10);
		number = number / 10;	
	}

	uint8_t j = 0;
	if (sign) buff[j++] = '-';

	while (i > 0)
	{
		buff[j++] = tmp_buff[--i];
	}
	buff[j] = '\0';
}

static uint16_t compute_uart_brr(uint32_t periph_clk, uint32_t baudrate)
{
	return ((periph_clk + (baudrate/2U))/baudrate);
}

static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate)
{
	USART2->BRR = compute_uart_brr(periph_clk, baudrate);
}
