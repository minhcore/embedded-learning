#include <stm32f446xx.h>
#include <stdint.h>

volatile uint16_t system_tick;

void delay_ms(uint16_t value)
{
    system_tick = 0;
    while (system_tick < value) {
        if (SysTick->CTRL & (0x01U << 16)) {
            system_tick++;
        }
    }
}

int main(void)
{

    SysTick->LOAD = 16000 - 1;
    SysTick->VAL = 0; // Clear value
    SysTick->CTRL |= 0x05; // Counter enable, processor clock (16 Mhz), 24 bit system timer

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable gpioA clock

    GPIOA->MODER &= ~(0x03U << 10); // delete PA5 values (MODER5)

    GPIOA->MODER |= 0x01U << 10; // GPIO mode

    GPIOA->BSRR = 0x01U << 5; // Set PA5 = 1

    system_tick = 0;

    while (1) {
        GPIOA->BSRR = (0x01U << (5 + 16)); // Reset PA5 = 0

        delay_ms(1000);

        GPIOA->BSRR = (0x01U << 5); // Set PA5 = 1

        delay_ms(1000);
    }
}
