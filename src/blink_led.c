#include <msp430.h>

#define LED0 BIT0

int main(void)
{
    WDTCTL = WDTHOLD + WDTPW; // Stop watchdog Timer

    if (CALBC1_1MHZ == 0xFF) {
        while (1)
            ;
    }

    P1DIR |= LED0;
    P1OUT |= LED0;

    while (1) {
        P1OUT ^= LED0;
        __delay_cycles(1000000);
    }
}