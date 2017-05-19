#include <msp430.h>     // zmienic header

//-----------------------------------------------------------------
// Inicjalizacja programu
void system_init(void)
{
	//Przyciski:
	//  7: start
	//  6: stopuj liczenie
 	//  5: start
	//  4: stopuj liczenie
  	//  3: start
	//  2: stopuj liczenie
  	//  1: start
	//  0: stopuj liczenie

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
  	//P4OUT |= BIT1+BIT2+BIT7;

　
	/* Insert your low-level initializations here */
	WDTCTL = WDTPW + WDTHOLD; // Stop Watchdog timer
	/*==================================*/
	/* Choose if segment initialization */
	/* should be done or not. */
	/* Return: 0 to omit initialization */
	/* 1 to run initialization */
	/*==================================*/
}

//-----------------------------------------------------------------

int main(void)
{
 	system_init();
	__enable_interrupt();

  	//P2OUT = 0x92;   // ustawienie 'liczby' na wyświetlaczu
 	// P2PUT = 0x6D;

	while(1)
	{
		P2OUT = ~P6IN;

	}

	return 0;
}

　
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
//  switch(!(x))
//         case P1IN & BIT0:  P3OUT |= BIT0;  break;

	if(!(P1IN & BIT0))
	{
    		P3OUT ^= BIT0;    // sygnał SA
    		P4OUT ^= BIT0;
    		P1IFG &= ~BIT0;
	}
  	else if(!(P1IN & BIT1))
	{
    		P3OUT ^= BIT1;    // sygnał SB
    		P4OUT ^= BIT1;
    		P1IFG &= ~BIT1;
	}
  	else if(!(P1IN & BIT2))
	{
    		P3OUT ^= BIT2;    // sygnał SC
    		P4OUT ^= BIT2;
    		P1IFG &= ~BIT2;
	}
  	else if(!(P1IN & BIT3))
	{
    		P3OUT ^= BIT3;    // sygnał SD
    		P4OUT ^= BIT3;
    		P1IFG &= ~BIT3;
	}
  	else if(!(P1IN & BIT4))
	{
    		P3OUT ^= BIT4;    // sygnał RBI
    		P4OUT ^= BIT4;
    		P1IFG &= ~BIT4;
	}
  	else if(!(P1IN & BIT5))
	{
    		P3OUT ^= BIT5;    // sygnał BI
    		P4OUT ^= BIT5;
    		P1IFG &= ~BIT5;
	}
  	else if(!(P1IN & BIT6))
	{
	    	P3OUT ^= BIT6;    // sygnał LT
	    	P4OUT ^= BIT6;
	    	P1IFG &= ~BIT6;
	}
  	else if(!(P1IN & BIT7))
	{
   	 		P3OUT ^= BIT7;    // sygnał DP
    		P4OUT ^= BIT7;
    		P1IFG &= ~BIT7;
	}
  	else
  		P1IFG &= 0x00;
}
