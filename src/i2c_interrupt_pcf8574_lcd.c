#include <msp430g2553.h>
#include <stdint.h>
#include <stdbool.h>

#define SCL_PIN BIT6
#define SDA_PIN BIT7

// A2_A1_A0 = 0x41 -> shift right 1 bit -> 0x20
#define PCF8574_SLAVE_ADDRESS 0x20

//  P7  <--->   DB7
//  P6  <--->   DB6
//  P5  <--->   DB5
//  P4  <--->   DB4
//  P3  <--->   Backlight
//  P2  <--->   E
//  P1  <--->   RW
//  P0  <--->   RS

typedef enum {
    SENDING,
    STOP
} i2c_state;

static volatile uint8_t byte;

static volatile i2c_state state;

static void pcf_sending(uint8_t data)
{
    while (UCB0STAT & UCBBUSY) { }
    state = SENDING;
    byte = data;
    UCB0CTL1 |= UCTXSTT; // start bit
    IE2 |= UCB0TXIE;
    __bis_SR_register(GIE + LPM0_bits); // go to sleep
}

static void lcd_send_byte(uint8_t byte, bool mode)
{
    uint8_t high_nibble = byte & 0xF0;
    uint8_t low_nibble = (byte << 4) & 0xF0;

    bool backlight = 1;
    bool rw = 0;
    bool enable = 0;

    // seding high nibble
    uint8_t data = high_nibble | (backlight << 3) | (0 << 2) | (rw << 1) | mode;
    pcf_sending(data);

    enable = 1;
    data |= (enable << 2);
    pcf_sending(data);

    data &= ~(enable << 2);
    pcf_sending(data);

    // sending low nibble
    data = low_nibble | (backlight << 3) | (0 << 2) | (rw << 1) | mode;
    pcf_sending(data);

    enable = 1;
    data |= (enable << 2);
    pcf_sending(data);

    data &= ~(enable << 2);
    pcf_sending(data);
}

static void lcd_send_nibble(uint8_t data, bool mode)
{
    uint8_t nibble = (data << 4) & 0xF0;
    bool backlight = 1;
    bool rw = 0;
    bool enable = 0;

    uint8_t value = nibble | (backlight << 3) | (enable << 2) | (rw << 1) | mode;
    pcf_sending(value);

    enable = 1;
    value |= (enable << 2);
    pcf_sending(value);

    value &= ~(enable << 2);
    pcf_sending(value);
}

static void lcd_send_string(char *p, bool mode)
{
    while (*p != '\0') {
        lcd_send_byte(*p, mode);
        p++;
    }
}

static void lcd_clear_display(void)
{
    lcd_send_byte(0x01, 0);
}

static void lcd_return_home(void)
{
    lcd_send_byte(0x02, 0);
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // stop Watch Dog Timer

    // safety check
    if (CALBC1_1MHZ == 0xFF) {
        while (1) { }
    }

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
    UCB0CTL1 |= UCSSEL_2 + UCTR; // SMCLK clock and choose transmitter
    UCB0BR0 = UCBRF_10; // SMCLK: 1Mhz/12 = 100 kHz
    UCB0BR1 = UCBRF_0;
    UCB0I2CSA = PCF8574_SLAVE_ADDRESS; // Set slave address
    // Configure ports
    P1SEL |= SCL_PIN + SDA_PIN;
    P1SEL2 |= SCL_PIN + SDA_PIN;
    // Clear UCSWRST
    UCB0CTL1 &= ~UCSWRST;
    // clear interrupt flag and enable NACK interrupt
    UCB0I2CIE |= UCNACKIE;
    IFG2 &= ~UCB0TXIFG;

    __bis_SR_register(GIE);

    // init lcd
    lcd_send_nibble(0x03, 0);
    __delay_cycles(5000); // delay 5ms (datasheet said 4.1 ms)

    lcd_send_nibble(0x03, 0);
    __delay_cycles(1000); // delay 1ms (datasheet said 100 us)

    lcd_send_nibble(0x03, 0);

    lcd_send_nibble(0x02, 0);

    // function set
    lcd_send_byte(0x28, 0); // 4 bit, 2 line, font 5x8

    // display on
    lcd_send_byte(0x0C, 0); // display on, cursor off, blink off

    // clear display
    lcd_send_byte(0x01, 0);

    // entry mode set
    lcd_send_byte(0x06, 0);

    while (1) {
        lcd_send_string("I2C Interrupt !!!", 1);
        __delay_cycles(2000000); // delay 2s
        lcd_clear_display();
        lcd_return_home();
    }
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void i2c_tx_isr(void)
{
    if (IFG2 & UCB0TXIFG) {
        switch (state) {
        case SENDING:
            UCB0TXBUF = byte; // load data into buffer
            state = STOP;
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
    }
}