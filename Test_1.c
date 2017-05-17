#include  <msp430g2553.h>       // zmienic header

//-----------------------------------------------------------------
// Inicjalizacja programu
int _system_pre_init(void)
{
	//Przyciski:
	//  0: start
	//  1: stopuj liczenie
	
    //	P1DIR &= ~BIT0 + ~BIT1;		// P1.0,1 -> input
	P1DIR &= 0x00; 			// Po prostu cały port1 jako input (zera w P1DIR)
	
	P1IE  |= BIT0;			// Odblokuj przerwania na P1.0
	P1IES |= BIT0;			// Ustaw zglaszanie przerwania na bicie 0 zboczem rosnacym
	
	P2DIR |= 0xFF;			//Port2 jako output dla diod

    
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
	
	//------------------------------ 
	// TO CHYBA MOŻNA W INIT
				// BCSCTL1 = Basic Clock System Control Register 1
	BCSCTL1 |= DIVA_3;	// DIVA_x odpowiada za dzielenie częstotliwości dla 
				// zegara ACLK, tutaj DIVA_3 = dzielenie przez 8
	
	BCSCTL3 |= XCAP_3;	//Oscillator capacitor selection - nie wiem po co to
				//12.5pF cap- setting for 32768Hz crystal
	//------------------------------
	
	
	while(1)
	{
		_BIS_SR(LPM3_bits + GIE);			// Enter LPM3
		__bis_SR_register(LPM3_bits + GIE); 		// <- alternatywnie
		
	}


	
    
	return 0;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	// START odliczania czasu aż do wciśnięcia przycisku 1
	P2OUT &= ~BIT0; 		// gaszenie diody 0
	
	// zatrzymanie Timer_A
	TACCTL0 = ~CCIE + ~CCIFG;		// TACCR0 interrupt disabled + CCIFG = 0
	TACTL = MC_0;				// MC_0 = Timer_A halt
	
	// Odpalenie Timer_B
	TBCCTL0 = CCIE;				// TBCCR0 interrupt enabled
	TBCCR0 = 511;				// po sekundzie będzie interrupt aby zmienic wyświetlane dane
	TBCTL = TASSEL_1 + ID_3 + MC_1;		// ACLK, /8, upmode
	
	P1IE  |= BIT1;				// Odblokuj przerwania na P1.0 aby móc zastopować liczenie
	
	
	//-------------------------------
	// Ustaw czas 0.0 na wyświetlaczu
	
	//-------------------------------
	
	
}

// Stoper
#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B (void)
{
	// nie wiem czy to co jest w timerze po doliczeniu do TBCCR0 się samo resetuje czy nie
	//edit: samo się powinno resetować
	TBCCR0 = 511;				// po sekundzie będzie interrupt aby zmienic to co na wyświetlaczu
	
	
	//--------------------------------
	// kod do zmiany na wyświetlaczu
	
	//--------------------------------
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if(!P1IN & ~BIT0) 	// Jeśli przycisk P1.0 wciśnięty   /chyba
	if(!(P1IN & BIT0)) 	// <- alternatywnie
	{
		P2OUT |= BIT0;				// zapal diode na potwierdzenie że weszło przerwanie
		TACCTL0 = CCIE;				// CCR0 interrupt enabled
		TACCR0 = 1023;				// 512 -> 1 sec, 30720 -> 1 min // odlicza do 2s
		TACTL = TASSEL_1 + ID_3 + MC_1;		// ACLK, /8, upmode
		P1IFG &= ~BIT0;				// Czyszczenie flagi IFG
		P1IE  &= ~BIT0;				// Zablokuj przerwania na P1.0
		// po naciścięciu 0 czekam 2 sekundy na pojawienie się sygnały start
	}
	if(P1IN & ~BIT1) 	// Jeśli przycisk P1.1 wciśnięty   /chyba
	if(!(P1IN & BIT1)) 	// <- alternatywnie
	{
	}
		
	
} 
