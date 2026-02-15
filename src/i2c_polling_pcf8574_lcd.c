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

static void pcf_sending(uint8_t data)
{
    UCB0CTL1 |= UCTXSTT; // Start
    while (!(IFG2 & UCB0TXIFG)) { } // wait for UCB0TXIFG to set
    UCB0TXBUF = data;
    while (!(IFG2 & UCB0TXIFG)) { } // wait for UCB0TXIFG to set
    UCB0CTL1 |= UCTXSTP; // Stop
    while (UCB0CTL1 & UCTXSTP) { } // wait for stop bit completed
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

    __delay_cycles(2000); // delay 2 ms
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

    __delay_cycles(2000); // delay 2ms
}

static void lcd_send_string(char *p, bool mode)
{
    while (*p != '\0') {
        lcd_send_byte(*p, mode);
        p++;
    }
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

    // init lcd
    lcd_send_nibble(0x03, 0);
    __delay_cycles(5000); // delay 5ms + 2ms in function (datasheet said 4.1 ms)

    lcd_send_nibble(0x03, 0);
    __delay_cycles(1000); // delay 1ms + 2ms in function (datasheet said 100 us)

    lcd_send_nibble(0x03, 0);

    lcd_send_nibble(0x02, 0);

    // function set
    lcd_send_byte(0x28, 0); // 4 bit, 2 line, font 5x8

    // display on
    lcd_send_byte(0x0C, 0); // display on, cursor off, blink off

    // clear display
    lcd_send_byte(0x01, 0); // delay 2ms already in function

    // entry mode set
    lcd_send_byte(0x06, 0);

    while (1) {
        lcd_send_string("I love my family", 1);
        __delay_cycles(2000000); // delay 2s
        lcd_send_byte(0x01, 0); // clear display
        lcd_send_byte(0x02, 0); // back to the line
        __delay_cycles(1000000); // delay 1s
    }
}