#ifndef UART_H_
#define UART_H_

#include "stm32f4xx.h"
#include "stdint.h"

#define SYS_FREQ 16000000

void uart_init(uint32_t baudrate);
void uart_write_char(char c);
void uart_write_string(char* c);
void uart_write_int(int32_t number);

#endif
