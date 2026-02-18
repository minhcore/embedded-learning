#include <msp430g2553.h>
#include <stdint.h>
#include <stdbool.h>

#define SCL_PIN BIT6
#define SDA_PIN BIT7
#define UART_TX_PIN BIT2

typedef enum {
    SENDING,
    STOP
} i2c_state;

volatile uint8_t raw_data[6];
volatile i2c_state state = SENDING;
volatile uint8_t tx_byte;
volatile uint8_t counter;
volatile uint8_t *pointer;
volatile bool tx_send_stop;
volatile uint16_t tick;

void i2c_write(uint8_t addr, uint8_t data)
{
    while (UCB0STAT & UCBBUSY) { } // wait for bus busy
    tx_byte = data;
    state = SENDING;
    UCB0I2CSA = addr; // set slave address
    __disable_interrupt();
    UCB0CTL1 |= UCTXSTT + UCTR; // send start bit and transmitter
    IE2 |= UCB0TXIE; // enable tx_interrupt
    IE2 &= ~UCB0RXIE; // disable rx_interrupt
    __bis_SR_register(LPM0_bits + GIE); // cpu goes to sleep
}

void i2c_read(uint8_t addr, uint8_t byte_count, volatile uint8_t *data)
{
    while (UCB0STAT & UCBBUSY) { }
    pointer = data;
    counter = byte_count;
    UCB0CTL1 &= ~UCTR; // receiver mode
    UCB0I2CSA = addr; // set slave address
    __disable_interrupt();
    if (byte_count == 1) {
        UCB0CTL1 |= UCTXSTT; // send start bit;
        while (UCB0CTL1 & UCTXSTT) { } // wait for start to drop
        UCB0CTL1 |= UCTXSTP; // send bit stop;

    } else {
        UCB0CTL1 |= UCTXSTT; // send start bit;
    }
    IE2 |= UCB0RXIE; // enable receive interrupt
    IE2 &= ~UCB0TXIE; // disable transmit interrupt
    __bis_SR_register(LPM0_bits + GIE);
}

void uart_write_char(char c)
{
    UCA0TXBUF = c; // load data into buffer
    IE2 |= UCA0TXIE; // enable uart_tx interrupt
    __bis_SR_register(LPM0_bits + GIE);
}

void uart_write_string(char *c)
{
    while (*c != '\0') {
        uart_write_char(*c);
        c++;
    }
}

int main(void)
{
    uint16_t previous_tick = 0;

    WDTCTL = WDTPW + WDTHOLD; // stop Watch Dog Timer

    // safety check
    if (CALBC1_8MHZ == 0xFF) {
        while (1) { }
    }

    BCSCTL1 = CALBC1_8MHZ;
    DCOCTL = CALDCO_8MHZ;

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
    UCB0BR0 = 80; // SMCLK: 8Mhz/80 = 100 kHz
    UCB0BR1 = 0;
    // Configure ports
    P1SEL |= SCL_PIN + SDA_PIN;
    P1SEL2 |= SCL_PIN + SDA_PIN;
    // Clear UCSWRST
    UCB0CTL1 &= ~UCSWRST;
    // clear interrupt flag and enable NACK interrupt
    UCB0I2CIE |= UCNACKIE;

    // timer A init
    TACTL |= TASSEL_2 + MC_1; // SMCLK, up mode
    TACCR0 = 7999; // 1ms tick
    TACCTL0 |= CCIE; // enable interrupt

    // uart tx init

    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 |= UCSSEL_2; // SMCLK clock

    // maximum tx error: -0.1% and 0%
    // 8 Mhz / 9600 = 833
    // UCA0BR0 + UCA0BR1 x 256
    UCA0BR0 |= 65;
    UCA0BR1 |= 3;
    UCA0MCTL |= UCBRS_2 + UCBRF_0;

    // configure ports
    P1SEL |= UART_TX_PIN;
    P1SEL2 |= UART_TX_PIN;

    UCA0CTL1 &= ~UCSWRST;

    _bis_SR_register(GIE);

    while (1) {
        /*
        i2c_write(0x68, 0x3B);
        i2c_read(0x68, 1, raw_data);
        __delay_cycles(10000);
    */
        if (tick - previous_tick >= 1000) { // 1000 tick = 1s
            uart_write_string("Hello world! \n\r");
            P1OUT ^= BIT0;
            previous_tick = tick;
        }
    }
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void i2c_uart_tx_isr(void)
{
    if ((IFG2 & UCB0TXIFG) && (IE2 & UCB0TXIE)) { // interrupt by transmiting i2c
        switch (state) {
        case SENDING:
            UCB0TXBUF = tx_byte; // load data into buffer
            state = STOP;
            break;
        case STOP:
            UCB0CTL1 |= UCTXSTP; // stop bit
            state = SENDING;
            IE2 &= ~UCB0TXIE; // disable interrupt to prevent loop isr (stop won't clear interrupt )
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        }
    } else if ((IFG2 & UCB0RXIFG) && (IE2 & UCB0RXIE)) { // interupt by recieving i2c
        *pointer++ = UCB0RXBUF;
        counter--;
        if (counter == 1) {
            while (UCB0CTL1 & UCTXSTT) { }
            UCB0CTL1 |= UCTXSTP; // send stop bit
        } else if (counter == 0) {
            __bic_SR_register_on_exit(LPM0_bits);
        }
    } else if ((IFG2 & UCA0TXIFG) && (IE2 & UCA0TXIE)) { // interrupt by transmiting uart
        IE2 &= ~UCA0TXIE; // disable tx interrupt;
        __bic_SR_register_on_exit(LPM0_bits);
    }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void i2c_error_isr(void)
{
    if (UCB0STAT & UCNACKIFG) {
        UCB0CTL1 |= UCTXSTP; // stop bit
        UCB0STAT &= ~UCNACKIFG; // clear flag
        state = SENDING;
        P1OUT |= BIT0;
        __bic_SR_register_on_exit(LPM0_bits);
        return;
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void timer_isr(void)
{
    tick++;
}