#include <msp430g2553.h>
#include <stdint.h>
#include <stdbool.h>

#define SCL_PIN BIT6
#define SDA_PIN BIT7
#define I2C_PIN_MASK (SCL_PIN | SDA_PIN)

void system_init(void);
void unused_pins_init(void);
void i2c_init(void);

int main(void)
{
    system_init();
    unused_pins_init();
    i2c_init();
    while (1) { }
}

void system_init(void)
{
    WDTCTL = WDTPW + WDTHOLD; // stop Watchdog Timer
    while (CALBC1_16MHZ == 0xFF) { } // safety check
    // configure 16 MHz clock
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
}

void unused_pins_init(void)
{
    P1DIR |= ~I2C_PIN_MASK;
    P1OUT |= ~I2C_PIN_MASK;
    P2DIR = 0xFF;
    P2OUT = 0x00;
}

void i2c_init(void)
{
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 |= UCMST + UCMODE_3; // master mode and i2c mode
    UCB0CTL1 |= UCSSEL_3; // SMCLK
    // 16 MHz / 40 = 400 KHz (fast mode)
    UCB0BR0 = 40;
    UCB0BR1 = 0;
    P1SEL |= BIT6 + BIT7;
    P1SEL2 |= BIT6 + BIT7;
    UCB0CTL1 &= ~UCSWRST;
    UCB0I2CIE |= UCNACKIE; // enable NACK interrupt
}
