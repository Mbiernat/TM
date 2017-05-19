#include <msp430.h>

void initTimerA();

void initTimerB();

void initMsp();

void count();

volatile char time = 8;// char because char has 8 bits, int has 16 bits
volatile char sec;
volatile char milisec;
unsigned char numbers[] = {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9};

void main(void)
{
 	initMsp();
	initTimerA();
	initTimerB();

	//P2OUT = 0xFE;
	sec = 0;
	milisec = 0;

	__enable_interrupt();

	while(1)
	{
		__enable_interrupt();
		//__bis_SR_register(LPM0 + GIE); 	// LPM0 with interrupts enabled

	//	P2OUT = ~0x02;
	//	P3OUT = numbers[sec];

		//P2OUT = ~0x01;
		//P3OUT = numbers[milisec];
	}
}

void initMsp()
{
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	//P4DIR |= 0xFF;					// Set P4.0 to output direction 8 bits data in dynamic display
	//P3DIR |= 0xFF;					// Set P3.0 to output direction 8 bits control in dynamic display+

	P1DIR &= 0x00; 			// Port1 - input - przyciski
	P6DIR &= 0x00;			// Port6 - input - nastawniki hex
	P1IE  |= 0xFF;			// Odblokuj przerwania na P1.0
	P1IES |= 0xFF;			// Ustaw zglaszanie przerwania na bicie 0 zboczem rosnacym

	P2DIR |= 0xFF;			//Port2 - output - wyświetlacz dane
	P3DIR |= 0xFF;			//Port3 - output - wyświetlacz sterowanie
	P4DIR |= 0xFF;     		//Port4 - output - diody

	P2OUT &= 0x00;
	P3OUT |= 0xFF;
	P4OUT &= 0x00;
}

void initTimerA()					// http://referencedesigner.com/blog/understanding-timers-in-ms430-2/2062/
{
	TACCTL0 = CCIE;
	TACCR0 = 32768; 					// to 1 sec, ACLK - 32768-Hz	sekundy
	TACCR1 = 3277;						// to 1/10 sec					setne
	TACTL |= TASSEL_1 + ID_0 + MC_1;// TASSEL_1 - ACLK, MC_1 - up mode count to CCR0, ID_0 is flag that ACLK counts with 32768Hz frequency, ID_1 1/2 previous frequency etc.
}

void initTimerB()					// http://referencedesigner.com/blog/understanding-timers-in-ms430-2/2062/
{
	TBCCTL0 = CCIE;
	TBCCR0 = 1092; 					// sekundy		30 razy na sek
	TBCCR1 = 546;					// setne		60 razy na sek
	TBCTL |= TASSEL_1 + ID_0 + MC_1;// TASSEL_1 - ACLK, MC_1 - up mode count to CCR0, ID_0 is flag that ACLK counts with 32768Hz frequency, ID_1 1/2 previous frequency etc.
}

// Timer A0 interrupt service routine
#pragma vector = TIMERA0_VECTOR
__interrupt void Timer_A(void)
{
	if(CCIFG)			// liczenie sekund
	{
		sec++;
		if(sec > 9 )
			sec = 0;
		P2OUT = ~0x02;
		P3OUT = numbers[sec];
	}
	else if(TACCTL1 & CCIFG)	// liczenie setnych
	{
		milisec++;
		if(milisec > 9 )
			milisec = 0;
	}

}

// Timer B0 interrupt service routine
#pragma vector = TIMERB0_VECTOR
__interrupt void Timer_B(void)
{
	if(TBCCTL0 & CCIFG)			// wyswietlanie sekund
	{
	//	P2OUT = ~0x02;
	//	P3OUT = numbers[sec];
	}
	else if(TBCCTL1 & CCIFG)	// wyswietlanie setnych
	{
		//P2OUT = ~0x01;
	//	P3OUT = numbers[milisec];
	}

}
