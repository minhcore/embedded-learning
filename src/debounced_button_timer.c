#include <msp430g2553.h>

#define LED0 BIT0
#define BUTTON BIT3

int main(void)
{
    WDTCTL = WDTHOLD + WDTPW; // stop WDT

    if (CALBC1_1MHZ == 0xFF) {
        while (1) { }
    }

    // init all unused pin to prevent floating
    P1DIR = 0xFF;
    P1OUT = 0x00;
    P2DIR = 0xFF;
    P2OUT = 0x00;

    // led init
    P1DIR |= LED0;
    P1OUT &= ~LED0;

    // button init
    P1DIR &= ~BUTTON;
    P1OUT |= BUTTON; // pull up selected
    P1REN |= BUTTON; // enable pull up
    P1IES |= BUTTON; // trigger when high-to-low transition
    P1IFG &= ~BUTTON; // clear button interrupt
    P1IE |= BUTTON; // enable interrupt

    // timer init
    TACTL &= ~TAIFG; // clear timer interrupt
    TACTL |= TASSEL_2 + MC_0; // choose SMCLK, stop mode
    TACCR0 = 14999; // set 15 ms
    __bis_SR_register(GIE + LPM0_bits);
    while (1) { }
}

#pragma vector = PORT1_VECTOR
__interrupt void button_isr(void)
{
    P1IE &= ~BUTTON; // disable interrupt
    P1IFG &= ~BUTTON; // clear interrupt flag
    TACTL |= MC_1 + TAIE; // Up mode: counts up to TACCR0
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void timer_isr(void)
{
    if ((P1IN & BUTTON) == 0) {
        P1OUT ^= LED0;
    }

    TACTL = TASSEL_2 + MC_0; // stop mode
    TACTL &= ~TAIFG; // clear interrupt flag
    TACTL &= ~TAIE; // disable interrupt

    P1IE |= BUTTON; // enable button interrupt
}
