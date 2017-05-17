#include  <msp430g2553.h>       // zmienic header

//-----------------------------------------------------------------
// Inicjalizacja programu
int _system_pre_init(void)
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
	P1IE  |= 0xFF;			// Odblokuj przerwania na P1.0
	P1IES |= 0xFF;			// Ustaw zglaszanie przerwania na bicie 0 zboczem rosnacym
	
	P2DIR |= 0xFF;			//Port2 - output - wyświetlacz dane
  P3DIR |= 0xFF;			//Port3 - output - wyświetlacz sterowanie
  P4DIR |= 0xFF;      //Port4 - output - diody

    
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
  P2OUT = 0x92;   // ustawienie 'liczby' na wyświetlaczu
 // P2PUT = 0x6D;
  
	while(1)
	{
    
		
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
  elseif(!(P1IN & BIT1))
	{
    P3OUT ^= BIT1;    // sygnał SB
    P4OUT ^= BIT1;
	}
  elseif(!(P1IN & BIT2))
	{
    P3OUT ^= BIT2;    // sygnał SC
    P4OUT ^= BIT2;
	}
  elseif(!(P1IN & BIT3))
	{
    P3OUT ^= BIT3;    // sygnał SD
    P4OUT ^= BIT3;
	}
  elseif(!(P1IN & BIT4))
	{
    P3OUT ^= BIT4;    // sygnał RBI
    P4OUT ^= BIT4;
	}
  elseif(!(P1IN & BIT5))
	{
    P3OUT ^= BIT5;    // sygnał BI
    P4OUT ^= BIT5;
	}
  elseif(!(P1IN & BIT6))
	{
    P3OUT ^= BIT6;    // sygnał LT
    P4OUT ^= BIT6;
	}
  elseif(!(P1IN & BIT7))
	{
    P3OUT ^= BIT7;    // sygnał DP
    P4OUT ^= BIT7;
	}
		
	
} 
