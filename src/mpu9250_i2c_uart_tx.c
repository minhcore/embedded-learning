#include <msp430g2553.h>
#include <stdint.h>
#include <stdbool.h>

#define SCL_PIN BIT6
#define SDA_PIN BIT7

typedef enum {
    SENDING,
    STOP
} i2c_state;

volatile raw_data[6];
volatile i2c_state state = SENDING;
volatile uint8_t tx_byte;
volatile uint8_t counter;
volatile uint8_t *pointer;
volatile bool tx_send_stop;

void i2c_write(uint8_t addr, uint8_t data, bool send_stop)
{
    while (UCB0STAT & UCBBUSY) { } // wait for bus busy
    tx_send_stop = send_stop;
    tx_byte = data;
    state = SENDING;
    UCB0I2CSA = addr; // set slave address
    UCB0CTL1 |= UCTXSTT + UCTR; // send start bit and transmitter
    IE2 |= UCB0TXIE; // enable tx_interrupt
    __bis_SR_register(LPM0_bits); // cpu goes to sleep
}

void i2c_read(uint8_t addr, uint8_t byte_count, uint8_t *data)
{

    pointer = data;
    counter = byte_count;
    UCB0CTL1 &= ~UCTR; // receiver mode
    UCB0I2CSA = addr; // set slave address
    if (byte_count == 1) {
        UCB0CTL1 |= UCTXSTT; // send start bit;
        while (!(UCB0CTL1 & UCTXSTT)) { } // wait for start to drop
        UCB0CTL1 |= UCTXSTT; // send bit stop;

    } else {
        UCB0CTL1 |= UCTXSTT; // send start bit;
    }
    IE2 |= UCB0RXIE; // enable receive interrupt
    __bis_SR_register(LPM0_bits);
}

int main(void)
{
    // init all unused pin to prevent floating
    P1DIR = 0xFF;
    P1OUT = 0x00;
    P2DIR = 0xFF;
    P2OUT = 0x00;

    // led error NACK init
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    // i2c init
    // set UCSWRST to initialize the USCI Module
    UCB0CTL1 |= UCSWRST;
    // Initialize all the USCI registers
    UCB0CTL0 |= UCSYNC + UCMST + UCMODE_3; // synchronous mode, master mode and USCI Mode: I2C
    UCB0CTL1 |= UCSSEL_2; // SMCLK clock
    UCB0BR0 = UCBRF_10; // SMCLK: 1Mhz/12 = 100 kHz
    UCB0BR1 = UCBRF_0;
    // Configure ports
    P1SEL |= SCL_PIN + SDA_PIN;
    P1SEL2 |= SCL_PIN + SDA_PIN;
    // Clear UCSWRST
    UCB0CTL1 &= ~UCSWRST;
    // clear interrupt flag and enable NACK interrupt
    UCB0I2CIE |= UCNACKIE;
    IFG2 &= ~UCB0TXIFG;

    __bis_SR_register(GIE);

    while (1) { }
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void i2c_tx_isr(void)
{
    if (IFG2 & UCB0TXIFG) {
        switch (state) {
        case SENDING:
            if (tx_send_stop) {
                state = STOP;
                UCB0TXBUF = tx_byte;
            } else {
                IE2 &= ~UCB0TXIE;
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case STOP:
            UCB0CTL1 |= UCTXSTP; // stop bit
            state = SENDING;
            IE2 &= ~UCB0TXIE; // disable interrupt to prevent loop isr (stop won't clear interrupt )
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        }
    }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void i2c_rx_isr(void)
{
    if (UCB0STAT & UCNACKIFG) {
        UCB0CTL1 |= UCTXSTP; // stop bit
        UCB0STAT &= ~UCNACKIFG; // clear flag
        state = SENDING;
        P1OUT |= BIT0;
        __bic_SR_register_on_exit(LPM0_bits);
        return;
    } else if (IFG2 & UCB0RXIFG) {
        *pointer++ = UCB0RXBUF;
        counter--;
        if (counter == 1) {
            UCB0CTL1 |= UCTXSTP; // send stop bit
        } else if (counter == 0) {
            __bic_SR_register_on_exit(LPM0_bits);
        }
    }
}