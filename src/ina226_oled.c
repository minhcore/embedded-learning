#include <msp430g2553.h>
#include <stdint.h>
#include <stdbool.h>

#define SCL_PIN BIT6
#define SDA_PIN BIT7
#define I2C_PIN_MASK (SCL_PIN | SDA_PIN)

#define INA226_ADDR 0x40

#define INA226_AVG_128 (0x04 << 9) // The number of samples that are collected and averaged

#define INA226_VBUSCT_1MS1 (0x04 << 6) // Sets the conversion time for the bus voltage measurement

#define INA226_VSHCT_1MS1 (0x04 << 3) // Sets the conversion time for the shunt voltage measurement

#define INA226_MODE_CONTINUOUS (0x07)

#define INA226_CONFIGURATION_REG (0x00)

#define INA226_CALIBRATION_REG (0x05)

#define INA226_BUS_VOLTAGE_REG (0x02)

#define INA226_POWER_REG (0x03)

#define INA226_CURRENT_REG (0x04)

#define OLED_ADDR 0x3C

typedef enum {
    I2C_IDLE,
    I2C_TX_DATA,
    I2C_RX_DATA,
    I2C_REPEAT_START,
    I2C_STOP
} i2c_state_t;

struct i2c_s
{
    volatile uint8_t *pointer;
    volatile uint8_t byte_cnt;
    volatile i2c_state_t state;
    volatile bool send_stop;
};

typedef enum {
    INA226_MODE_800MA,
    INA226_MODE_150MA
} ina226_mode_t;

struct ina226_mode_config_s
{
    uint16_t cal;
    uint16_t lsb_ua;
};

const struct ina226_mode_config_s mode_table[] = {
    { 0x0800, 25 }, // 800 mA
    { 0x2800, 5 }, // 150 mA
    { 0xC800, 1 } // 30 mA
};

typedef enum {
    OLED_STATE_DRAW,
    OLED_STATE_CLEAR
} oled_state_t;

oled_state_t oled_state = OLED_STATE_DRAW;

struct i2c_s i2c;
volatile uint16_t system_tick;
volatile uint16_t oled_prev_tick;
volatile ina226_mode_t ina226_current_mode;

void system_init(void);
void unused_pins_init(void);
void i2c_bus_recovery(void);
void i2c_init(void);
void i2c_read(uint8_t addr, uint8_t byte_cnt, uint8_t *data, bool stop);
void i2c_write(uint8_t addr, uint8_t byte_cnt, uint8_t *data, bool stop);
void ina226_set_mode(ina226_mode_t mode);
int16_t ina226_read_voltage_raw(void);
int16_t ina226_read_current_raw(void);
int16_t ina226_read_power_raw(void);
void oled_init(void);
void oled_clear_display(void);
void oled_square_draw(void);

int main(void)
{
    system_init();
    // i2c_bus_recovery();
    i2c_init();
    unused_pins_init();
    // ina226_set_mode(INA226_MODE_150MA);
    oled_init();
    oled_prev_tick = 0;
    __enable_interrupt(); // for system tick (timera0)

    /*
    uint8_t reg = INA226_CALIBRATION_REG;
    uint8_t buf[2];
    i2c_write(INA226_ADDR, 1, &reg, false);
    i2c_read(INA226_ADDR, 2, buf, true);

    int16_t buffer[3];
    buffer[0] = ina226_read_voltage_raw();
    buffer[1] = ina226_read_current_raw();
    buffer[2] = ina226_read_power_raw();
    */
    while (1) {
        if (system_tick - oled_prev_tick >= 1000) {
            oled_prev_tick = system_tick;
            switch (oled_state) {
            case OLED_STATE_DRAW:
                oled_square_draw();
                oled_state = OLED_STATE_CLEAR;
                break;
            case OLED_STATE_CLEAR:
                oled_clear_display();
                oled_state = OLED_STATE_DRAW;
                break;
            }
        }
    }
}

void system_init(void)
{
    WDTCTL = WDTPW + WDTHOLD; // stop Watchdog Timer
    while (CALBC1_16MHZ == 0xFF) { } // safety check
    // configure 16 MHz clock
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    // timer init (system tick)
    TACTL |= TASSEL_2 + MC_1; // SMCLK and Up mode
    TACCTL0 |= CCIE; // Enable capture/compare interrupt
    TACCR0 = 15999; // 16 000 -> 1 ms (one tick every 1 ms)
}

void unused_pins_init(void)
{
    P1DIR |= ~I2C_PIN_MASK;
    P1OUT &= I2C_PIN_MASK;
    P2DIR = 0xFF;
    P2OUT = 0x00;
}

void i2c_bus_recovery(void)
{
    P1SEL &= ~I2C_PIN_MASK;
    P1SEL2 &= ~I2C_PIN_MASK;
    P1DIR |= I2C_PIN_MASK; // Output
    P1OUT |= I2C_PIN_MASK; // SCL=1, SDA=1

    if (!(P1IN & SDA_PIN)) {
        uint8_t i;
        P1DIR &= ~SDA_PIN;
        for (i = 0; i < 9; i++) {
            P1OUT &= ~SCL_PIN;
            __delay_cycles(40);
            P1OUT |= SCL_PIN;
            __delay_cycles(40);
            if (P1IN & SDA_PIN)
                break;
        }

        P1DIR |= SDA_PIN;
        P1OUT &= ~SDA_PIN;
        __delay_cycles(40);
        P1OUT |= SDA_PIN;
    }
    __delay_cycles(160); // Stability
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
    __delay_cycles(160);
    UCB0I2CIE |= UCNACKIE; // enable NACK interrupt

    i2c.state = I2C_IDLE;
}

void i2c_read(uint8_t addr, uint8_t byte_cnt, uint8_t *data, bool stop)
{
    if (i2c.state == I2C_IDLE) {
        uint16_t timeout = 1000;
        while ((UCB0STAT & UCBBUSY) && --timeout) {
            __delay_cycles(160);
        }
        if (timeout == 0) {
            i2c_bus_recovery();
            i2c_init();
            return;
        }
    }
    UCB0I2CSA = addr;
    i2c.byte_cnt = byte_cnt;
    i2c.pointer = data;
    i2c.send_stop = stop;
    UCB0CTL1 &= ~UCTR; // Receiver mode
    if (i2c.state == I2C_IDLE || i2c.state == I2C_REPEAT_START) {
        __disable_interrupt();
        if (byte_cnt == 1) {
            UCB0CTL1 |= UCTXSTT; // Send start bit + addr
            while (UCB0CTL1 & UCTXSTT) { } // In this case, trade off speed for safety (wait for start bit to drop)
            UCB0CTL1 |= UCTXSTP; // Send stop bit immediately
            *i2c.pointer = UCB0RXBUF;
            i2c.state = I2C_IDLE;
            __enable_interrupt();
        } else if (byte_cnt > 1) {
            UCB0CTL1 |= UCTXSTT; // Send start bit + addr
            IE2 |= UCB0RXIE; // Enable I2C RX interrupt
            i2c.state = I2C_RX_DATA;
            __bis_SR_register(LPM0_bits + GIE);
        }
    }
}

void i2c_write(uint8_t addr, uint8_t byte_cnt, uint8_t *data, bool stop)
{
    if (i2c.state == I2C_IDLE) {
        uint16_t timeout = 1000;
        while ((UCB0STAT & UCBBUSY) && --timeout) {
            __delay_cycles(160);
        }
        if (timeout == 0) {
            i2c_bus_recovery();
            i2c_init();
            return;
        }
    }
    UCB0I2CSA = addr;
    i2c.byte_cnt = byte_cnt;
    i2c.pointer = data;
    i2c.send_stop = stop;
    UCB0CTL1 |= UCTR; // Tranmisting mode
    if (i2c.state == I2C_IDLE || i2c.state == I2C_REPEAT_START) {
        __disable_interrupt();
        UCB0CTL1 |= UCTXSTT; // Send start bit + addr
        IE2 |= UCB0TXIE; // Enable TX interrupt
        i2c.state = I2C_TX_DATA;
        __bis_SR_register(LPM0_bits + GIE);
    }
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void i2c_tx_rx_isr(void)
{
    if ((IE2 & UCB0RXIE) && (IFG2 & UCB0RXIFG)) { // Interrupt by I2C RX
        switch (i2c.state) {
        case I2C_RX_DATA:
            *i2c.pointer++ = UCB0RXBUF;
            i2c.byte_cnt--;
            if (i2c.byte_cnt == 1) {
                if (i2c.send_stop == 1) {
                    UCB0CTL1 |= UCTXSTP; // Send bit stop
                } else {
                    i2c.state = I2C_REPEAT_START;
                }
            } else if (i2c.byte_cnt == 0) {
                i2c.state = I2C_IDLE;
                IE2 &= ~UCB0RXIE; // Disable RX interrupt
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case I2C_REPEAT_START:
            *i2c.pointer++ = UCB0RXBUF; // Read last byte
            IE2 &= ~UCB0RXIE; // Disable RX interrupt
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        }
    } else if ((IE2 & UCB0TXIE) && (IFG2 & UCB0TXIFG)) { // Interrupt by I2C TX
        switch (i2c.state) {
        case I2C_TX_DATA:
            UCB0TXBUF = *i2c.pointer++;
            i2c.byte_cnt--;
            if (i2c.byte_cnt == 0) {
                if (i2c.send_stop == 1) {
                    i2c.state = I2C_STOP;
                } else {
                    i2c.state = I2C_REPEAT_START;
                }
            }
            break;
        case I2C_STOP:
            UCB0CTL1 |= UCTXSTP; // Send bit stop
            i2c.state = I2C_IDLE;
            IE2 &= ~UCB0TXIE; // Disable TX interrupt
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        case I2C_REPEAT_START:
            IE2 &= ~UCB0TXIE; // Disable TX interrupt
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        }
    }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void i2c_nack_isr(void)
{
    if (UCB0STAT & UCNACKIFG) {
        if (i2c.state != I2C_RX_DATA) {
            UCB0CTL1 |= UCTXSTP; // Send stop bit
            UCB0STAT &= ~UCNACKIFG; // Clear NACK flag
            IE2 &= ~UCB0TXIE; // Disable TX interrupt
            i2c.state = I2C_IDLE;
            __bic_SR_register_on_exit(LPM0_bits);
        }
    }
}

void ina226_set_mode(ina226_mode_t mode)
{
    ina226_current_mode = mode;
    uint16_t config = INA226_AVG_128 | INA226_VBUSCT_1MS1 | INA226_VSHCT_1MS1 | INA226_MODE_CONTINUOUS;
    uint8_t buf[6];
    buf[0] = INA226_CONFIGURATION_REG;
    buf[1] = (config >> 8) & 0xFF;
    buf[2] = config & 0xFF;
    i2c_write(INA226_ADDR, 3, buf, true); // Configuration Register of INA226
    __delay_cycles(16000000); // delay 1s for stability
    buf[0] = INA226_CALIBRATION_REG;
    buf[1] = (mode_table[mode].cal >> 8) & 0xFF;
    buf[2] = mode_table[mode].cal & 0xFF;
    i2c_write(INA226_ADDR, 3, buf, true); // Calibration Register of INA226
    __delay_cycles(16000000); // delay 1s for stability
}

int16_t ina226_read_voltage_raw(void)
{
    uint8_t reg;
    uint8_t buf[2];
    reg = INA226_BUS_VOLTAGE_REG;
    i2c_write(INA226_ADDR, 1, &reg, false);
    i2c_read(INA226_ADDR, 2, buf, true);
    return (((int16_t)buf[0] << 8) | buf[1]);
}

int16_t ina226_read_current_raw(void)
{
    uint8_t reg;
    uint8_t buf[2];
    reg = INA226_CURRENT_REG;
    i2c_write(INA226_ADDR, 1, &reg, false);
    i2c_read(INA226_ADDR, 2, buf, true);
    return (((int16_t)buf[0] << 8) | buf[1]);
}

int16_t ina226_read_power_raw(void)
{
    uint8_t reg;
    uint8_t buf[2];
    reg = INA226_POWER_REG;
    i2c_write(INA226_ADDR, 1, &reg, false);
    i2c_read(INA226_ADDR, 2, buf, true);
    return (((int16_t)buf[0] << 8) | buf[1]);
}

void oled_init(void)
{
    uint8_t buf[18] = {
        0x00, // Control Byte
        0xa8, 0x3f, // Set Multiplex Ratio
        0xd3, 0x00, // Set Display Offset (0)
        0x40, // Set Display Start Line (0)
        0xa1, // Set Segment Re-map
        0xc8, // Set COM Output Scan Direction
        0xda, 0x12, // Set COM Pins Hardware Configuration
        0x81, 0x7f, // Set Contrast Control
        0xa4, // Set Normal Display
        0xd5, 0x80, // Set Oscillator Frequency
        0x8d, 0x14, // Charge Pump Setting: Enable
        0xaf // Set Display ON
    };
    i2c_write(OLED_ADDR, 18, buf, true);
}

void oled_clear_display(void)
{
    uint8_t buf_command[9] = {
        0x00, // Control Byte
        0x20, // Set Memory Adressing Mode
        0x00, // Horizontal Mode
        0x21, // Set Column Address
        0, // Start Column: 0
        127, // End Column: 127
        0x22, // Set Page Address
        0, // Start Page: 0
        7 // Stop Page: 7
    };
    uint8_t buf_clear[9] = { 0x40, // Data byte
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    i2c_write(OLED_ADDR, 9, buf_command, true);

    uint8_t i;
    for (i = 0; i < 127; i++) {
        i2c_write(OLED_ADDR, 9, buf_clear, false);
    }
    i2c_write(OLED_ADDR, 9, buf_clear, true);
}

void oled_square_draw(void)
{
    uint8_t command_buffer[] = {
        0x00, // Command Byte
        0x20, 0x00, // Set Memory Addressing Mode = Horizontal
        0x21, 56,   71, // Set Columns Address (0~127)
        0x22, 4,    5, // Set Page Address (0~7)
    };
    i2c_write(OLED_ADDR, 9, command_buffer, true);
    uint8_t i;
    uint8_t data_buffer[] = { 0x40, // Control Byte
                              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    for (i = 0; i < 7; i++) {
        i2c_write(OLED_ADDR, 9, data_buffer, false);
    }
    i2c_write(OLED_ADDR, 9, data_buffer, true);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void system_tick_isr(void)
{
    system_tick++;
}
