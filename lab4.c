#include  <msp430g2553.h>       // zmienic header


#define LED0	BIT0
#define LED1	BIT1



int _system_pre_init(void)
{
    
    P1DIR |= LED0;				// P1.0 (LED0) -> Output
    P1DIR |= LED1;				// P1.1 (LED1) -> Output
    // itd.
    
    P1DIR &= ~BIT3;				// P1.3 (SW2) -> Input
    
    /* Insert your low-level initializations here */
    WDTCTL = WDTPW + WDTHOLD; // Stop Watchdog timer
    /*==================================*/
    /* Choose if segment initialization */
    /* should be done or not. */
    /* Return: 0 to omit initialization */
    /* 1 to run initialization */
    /*==================================*/
}

int main(void)
{

	BCSCTL1 |= DIVA_3;				// ACLK/8
	BCSCTL3 |= XCAP_3;				//12.5pF cap- setting for 32768Hz crystal

	CCTL0 = CCIE;					// CCR0 interrupt enabled
	CCR0 = 511;					// 512 -> 1 sec, 30720 -> 1 min
	TACTL = TASSEL_1 + ID_3 + MC_1;			// ACLK, /8, upmode

	_BIS_SR(LPM3_bits + GIE);			// Enter LPM3 w/ interrupt
    
    return 0;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	P1OUT ^= BIT0;					// Toggle LED
}
