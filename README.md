# Embedded Learning

This repository documents my personal process of learning embedded systems.

## Description
I am currently exploring the MSP430 architecture.
This repo contains my raw code, failed experiments, and notes as I try to understand how the hardware works from scratch.

## Status
* **Stage**: Beginner / Learning
* **Goal**: To write bare-metal drivers without relying on existing libraries.

## MSP430 Learning Log
- [x] **01:** Blinking LED
- [x] **02:** Debounce button using Timer and Interrupt
- [x] **03:** Display LCD by using PCF8574 (I/O expander) and I2C polling
  <p align="center">
  <img src="msp430/docs/i2c_polling_pcf8574_lcd_.jpg" width="650">
</p>

- [x] **04:** Display LCD by using PCF8574(I/O expander) and I2C interrupt 
- [x] **05:** Reading accelerator value from mpu6500 using I2C and UART
<p align="center">
  <img src="msp430/docs/mpu_6500_terminal.png" height="400">
  &nbsp;&nbsp;
  <img src="msp430/docs/mpu_6500_breadboard.jpg" height="400">
</p>

- [x] **06:** Reading U, I and P using INA226 and OLED 1306
  <p align="center">
  <img src="msp430/docs/ina226_oled1306_breadboard_.jpg" width="650">
</p>

---
*Minh*