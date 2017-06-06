//#include <msp430.h>
//
//void initUart();
//void initSystem();
//void displayValue();
//
//char volatile receivedData;
//const short int refresh = 64; //  32678Hz
//
//void initUart()
//{
////	while(!(UTXE1 & UTXIFG1));
////    // zewnetrzny kwarc zeby dzialal bardzo szybko
////    // BCSCTL1 &= ~XT2OFF; // XT2 on
////    //BCSCTL2  = SELS; // wybiera XT2CLK bazujący na zewnętrznym kwarcu i daje na SMCLK source XT2CLK
////
////	U0BR1 = 0x00;
////	U0BR0 = 0x03;
////	U0MCTL = 0x4A;
////
////    //piny 3.4 i  3.5 jako we/wy TX/RX
////    P3SEL |= 0x30; // druga funkcja pinow
////
////    U0CTL = SWRST | CHAR; // USART control register, 271 manual
////    U0TCTL |= SSEL0; // ACLK
////    ME1 = URXE0 | UTXE0; // mozna odbierac  module enable register
////    U0CTL &= ~SWRST;
////
////    IE1 |= URXIE0 | UTXIE0; // przerwania od odbierania
//
//	UCTL0 |= SWRST;
//
//    TACTL = MC_1 | ID_0 | TASSEL_1; //clock init ACLK, mode Up, no divider.
//    BCSCTL1 &= ~XTS; //'slow mode' ACLK
//    TACCR0=refresh;
//
//    BCSCTL2 |= SELS;// Korzystamy z zewnetrznego kwarcu jako zrodla taktowania (XT2CLK: Optional high-frequency oscillator 450-kHz to
////  8-MHz range. )
//    P3SEL |= 0x30; //ustawiamy pierwsza i druga nozke jako Rx i Tx // P3.4 UTXD0 oraz P3.5 URDX0
//    ME1 |= UTXE0 + URXE0;// Wlaczamy modul do wysylania danych i odbierania w UART
//    UCTL0 |= CHAR;//PEV + PENA + CHAR + SPB;// Ustawiamy bit parzystosci, dlugosc danych 8 bitow, i 2 bity stopu
//
//// Ustawienia dla 115200Hz (dla UART: max 115200 hz)
//    UTCTL0 |= SSEL1; //typical 1,048,576-Hz SMCLK. --> LPM1
//    UBR00 = 0x40; // pierwszy dzielnik czestotliwosci
//    UBR10 = 0x00; // drugi dzielnik
//    UMCTL0 = 0x00;
//    UCTL0 &= ~SWRST; // wylaczenie software reset
//
//   TACCTL0 |= CCIE; //clock interrupts enabled
//   IE1 |= URXIE0; //przerwania wlaczone dla odbierania/dla wysylania
//   IE1 |= UTXIE0;
//}
//
//void initSystem()
//{
//    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
//
//    P4DIR = 0xFF;
//    P4OUT = 0x04;
//}
//
//void displayValue()
//{
//    P4OUT = receivedData;
//}
//
//void main(void)
//{
//    initSystem();
//    initUart();
//
//    while(1)
//    {
//    	TXBUF0 = 0x02;
//    	RXBUF0 = 0x02;
//        __bis_SR_register(GIE);
//
//        displayValue();
//    }
//}
//
//#pragma vector = USART0RX_VECTOR
//__interrupt void receiveData(void)
//{
//    receivedData = U0RXBUF;
//	//U0RXBUF = U0TXBUF;
//
//    LPM1_EXIT;
//}
//
//#pragma vector = USART0TX_VECTOR
//__interrupt void sendData(void)
//{
//    receivedData = 0x08;
//
//    LPM1_EXIT;
//}

#include <msp430.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headers/lab4.h"

enum TIMER_A_INTERR_MODE {RANDOMTIME, COUNT};
enum P1_INTERR_MODE {SWITCH_ON, STOP_TIMER, SWITCH_OFF};

// Inicjalizacja programu
void initSystem()
{
	WDTCTL = WDTPW + WDTHOLD; 	// Stop Watchdog timer

	// Output
	P6DIR |= 0xFF;			// Port2 - output - wyswietlacz dane
	P5DIR |= 0xFF;			// Port3 - output - wyswietlacz sterowanie
	P4DIR |= 0xFF;     		// Port4 - output - diody

	P6OUT = 0x00;			// Gaszenie wszystkich segmentów wyswietlacza
	P5OUT = 0x00;			//
	P4OUT = 0x00;			// Gaszenie diod

}

void initUart()
{

	UCTL0 |= SWRST;

	TACTL = MC_1 | ID_0 | TASSEL_1; //clock init ACLK, mode Up, no divider.
	BCSCTL1 &= ~XTS; //'slow mode' ACLK
	TACCR0=refresh;

	BCSCTL2 |= SELS;// Korzystamy z zewnetrznego kwarcu jako zrodla taktowania (XT2CLK: Optional high-frequency oscillator 450-kHz to
//  8-MHz range. )
	P3SEL |= 0x30; //ustawiamy pierwsza i druga nozke jako Rx i Tx // P3.4 UTXD0 oraz P3.5 URDX0
	ME1 |= UTXE0 + URXE0;// Wlaczamy modul do wysylania danych i odbierania w UART
	UCTL0 |= CHAR;//PEV + PENA + CHAR + SPB;// Ustawiamy bit parzystosci, dlugosc danych 8 bitow, i 2 bity stopu

// Ustawienia dla 115200Hz (dla UART: max 115200 hz)
	UTCTL0 |= SSEL1; //typical 1,048,576-Hz SMCLK. --> LPM1
	UBR00 = 0x40; // pierwszy dzielnik czestotliwosci
	UBR10 = 0x00; // drugi dzielnik
	UMCTL0 = 0x00;
	UCTL0 &= ~SWRST; // wylaczenie software reset

   TACCTL0 |= CCIE; //clock interrupts enabled
   IE1 |= URXIE0; //przerwania wlaczone dla odbierania/dla wysylania
   IE1 |= UTXIE0;
}

void initTimers()
{
	TACTL |= TASSEL_1 + ID_1 + MC_1;	// 16384 Hz -> 1s
	TBCTL |= TBSSEL_1 + ID_0 + MC_1;	//32768 Hz -> 1s

	CLOCK_A_HZ = 16384;
	CLOCK_B_HZ = 32768;

	seconds = miliseconds = 0;

	setTimer_A2();

	srand(time(NULL));
}


void resetVariables()
{
	seconds = refresh_id = word_id = numOfButton = miliseconds = 0;
	P4OUT = P1IFG = 0x00;
	P1IE = 0x80;
}

//-----------------------------------------------------------------

void main(void)
{
	initSystem();
	initTimers();
	initUart();

	getRandomData();

	while(true)
	{
		U0RXBUF = 
		__enable_interrupt();
		__bis_SR_register(GIE);
	}
}

#pragma vector = USART0RX_VECTOR
__interrupt void receiveData(void)
{
	char button = U0RXBUF;

	P4OUT = 5;

	//if(button == '1' && (P1_INTERR_MODE == 0))		// Poczatkowe uruchamianie licznika - od tego momentu liczenie losowych sekund
	if(P1_INTERR_MODE == 0)
	{
		getRandomData();
		setTimer_A0();

		IE1 &= ~URXIE0;

		prepareCounterB_ForRefresh();

		word_id = 0;

		P1_INTERR_MODE = STOP_TIMER;
	}
//	else if(button == ' ' && P1_INTERR_MODE == 1)
	else if(P1_INTERR_MODE == 1)
	{
		resetTimer(&TACCTL0);
		IE1 |= URXIE0;

		P1_INTERR_MODE = 3;

		if(seconds < 2)
			word_id = 1;
		else
			word_id = 2;
	}
//	else if( button == 'r' && (P1_INTERR_MODE == 3))
	else if(P1_INTERR_MODE == 3)
	{
		resetVariables();


		resetTimer(&TBCCTL0);
		P6OUT = 0x00;

		P1_INTERR_MODE = SWITCH_ON;

		IE1 |= URXIE0;

		setTimer_A2();

		word_id = 0;

		LPM3_EXIT; // chcemy wyeliminowac drgania stykow zeby nie przeszedl od razu do pierwszego if w tym przerwaniu
	}
}

void setTimer_A0()
{
	TACCTL0 &= ~CCIFG; // resey flagi przerwan
	TIMER_A_INTERR_MODE = RANDOMTIME; //
	TACCR0 = CLOCK_A_HZ * 2;			// Licznik liczy do tego losowego czasu
	TACCTL0 |= CCIE;					// Odblokowanie przerwan Timer_A0
}

void setTimer_A2()
{
	TACCTL0 &= ~CCIFG;
	TACCR0 = CLOCK_A_HZ/ 4;				// Licznik liczy do tego losowego czasu
	TACCTL0 |= CCIE;					// Odblokowanie przerwan Timer_A0
}

void prepareCounterB_ForRefresh()
{
	// Timer B do wyswietlania - odswiezanie diod
	TBCCR0 = CLOCK_B_HZ / 880;			// Licznik liczy co 1/440 sekundy
	TBCCTL0 |= CCIE;					// Odblokowanie przerwan Timer_B0
}

void prepareTimerA_ForCount()
{
	// Timer A0 do liczenia
	TIMER_A_INTERR_MODE = COUNT;			// Zmiana roli dla Timer_A0
	TACCR0 = CLOCK_A_HZ / 100;			// Licznik liczy co 1/100 sekundy
	TACCTL0 |= CCIE;					// Odblokowanie przerwan Timer_A1
	// Odblokowanie przycisku do zatrzymywania
}

// Timer A0 odlicza poczatkowy losowy czas
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	if(TIMER_A_INTERR_MODE == RANDOMTIME)		// TIMER A0
	{
		TACCTL0 &= ~CCIE;	// Blokuje przerwania Timer_A0
		prepareTimerA_ForCount();
		IE1 |= URXIE0;
	}
	else if(TIMER_A_INTERR_MODE == COUNT)							// TIMER A1
	{
		if(++miliseconds > 99)
		{
			miliseconds = 0;
			if(++seconds >= 10)
			{
				resetTimer(&TACCTL0); // zeby nie liczyl w nieskonczonosc jak ktos odszedl od miernika
				IE1 |= URXIE0;
				P1_INTERR_MODE = 3;
				word_id = 3;
			}
		}

		TACCTL1 &= ~CCIFG;	// Resetuje flage przewania
	}

	TACCTL0 &= ~CCIFG;	// Resetuje flage przerwania
}

// Timer B0 do odswiezania liczb na ekranie
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B0 (void)
{
	switch(refresh_id)
	{
		case 0:		// Odswiez setne sekundy
		{
			refreshDisplay(miliseconds % 10);
			break;
		}
		case 1:		// Odswiez dziesietne sekundy
		{
			refreshDisplay(miliseconds/10);	// Jesli miliseconds = 48 dostajemy num = 4
			break;
		}
		case 2:		// Odswiez sekundy
		{
			refreshDisplay(seconds % 10); // Jesli seconds = 48 dostajemy num = 8
			P5OUT |= 0x80;						// Dodanie kropki po liczbie sekund
			break;
		}
		case 3:		// Odswiez dziesiatki sekund
		{
			refreshDisplay(seconds / 10); // Jesli seconds = 48 dostajemy num = 4
			break;
		}
		case 4:
		{
			switch(word_id)
			{
				case 0:	refreshDisplay(10); break;
				case 1: refreshDisplay(13); break;
				case 2: refreshDisplay(10); break;
				case 3:	refreshDisplay(15); break;
			}
			break;
		}
		case 5:
		{
			switch(word_id)
			{
				case 0:	refreshDisplay(10); break;
				case 1: refreshDisplay(12); break;
				case 2: refreshDisplay(13); break;
				case 3: refreshDisplay(17); break;
			}
			break;
		}
		case 6:
		{
			switch(word_id)
			{
				case 0:	refreshDisplay(10); break;
				case 1: refreshDisplay(12); break;
				case 2: refreshDisplay(15); break;
				case 3: refreshDisplay(16); break;
			}
			break;
		}
		case 7:
		{
			switch(word_id)
			{
				case 0:	refreshDisplay(10); break;
				case 1: refreshDisplay(11); break;
				case 2: refreshDisplay(14); break;
				case 3: refreshDisplay(13); break;
			}
			refresh_id = 0;
			break;
		}
	}
}

void refreshDisplay(int segment)
{
	P6OUT = 0;
	P5OUT = displayedSymb[segment];
	P6OUT = numOfDisplay[refresh_id++];
}

void getRandomData()
{
	randomTime = (1 + (rand() % 5)) * CLOCK_A_HZ;		// otrzymamy czas w sekundach, Trzeba pomnozyc czas w sekundach zeby otrzymac w Hz dla zegara
	numOfButton = rand() % 7;		// Przyciski do stopowania od 0 do 6
}

void resetTimer(volatile unsigned int *timer)
{
	// Zerowanie timera B0
	*timer = OUTMOD_5;	// Resetuje Timer_B0
	*timer &= ~CCIE;	// Blokuje przerwania Timer_B0
	*timer &= ~CCIFG;	// Resetuje flage przewania
}

