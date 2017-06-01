#include <msp430.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//-----------------------------------------------------------------
void getRandomData();

void initTimers();

void initSystem();

//-----------------------------------------------------------------
volatile int CLOCK_A_HZ;		// Do przechowania aktualnej czestosci zegara
volatile long CLOCK_B_HZ;

volatile int numOfButton; 	// Numer przycisku, który trzeba wcisnac
unsigned const int BUTTONS [] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};	// Kody przycisku do zatrzymywania stopera

volatile long randomTime;	// Losowy czas, po którym odpalamy stoper

volatile int seconds;		// Sekundy liczone przez stoper
volatile int miliseconds;	// Setne liczone przez stoper

volatile int refresh_id;	// Do wybierania który segment wyswietlacza odswiezamy
unsigned const int numOfDisplay[] = {0xFE, 0xFD, 0xFB, 0xF7};						// Do wyboru segmentu wyswietlacza
unsigned const int displayedNum[] = {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9};	// Do wyswietlania konkretnego numeru
volatile int num;		// Zmienna pomocnicza

volatile int TIMER_A_INTERR_MODE;
volatile int P1_INTERR_MODE;

volatile bool oscilateFlag = false;
enum TIMER_A_INTERR_MODE {RANDOMTIME, COUNT, WAIT_FOR_ACTIVATION};
enum P1_INTERR_MODE {SWITCH_ON, STOP_TIMER, SWITCH_OFF};

// Inicjalizacja programu
void initSystem()
{
	WDTCTL = WDTPW + WDTHOLD; 	// Stop Watchdog timer

	// Input
	P1DIR &= 0x00; 			// Port1 - input - przyciski
	P1IE  |= BIT7;			// Odblokuj przerwania na P1.7
	P1IES |= 0xFF;			// Ustaw zglaszanie przerwania zboczem rosnacym

	// Output
	P2DIR |= 0xFF;			// Port2 - output - wyswietlacz dane
	P3DIR |= 0xFF;			// Port3 - output - wyswietlacz sterowanie
	P4DIR |= 0xFF;     		// Port4 - output - diody

	P2OUT |= 0xFF;			// Gaszenie wszystkich segmentów wyswietlacza
	P3OUT |= 0xFF;			//
	P4OUT &= 0x00;			// Gaszenie diod
}

void initTimers()
{
	TACTL |= TASSEL_1 + ID_1 + MC_1;	// 16384 Hz -> 1s
	TBCTL |= TBSSEL_1 + ID_0 + MC_1;	//32768 Hz -> 1s

	TACCTL0 &= ~CCIFG; 		// reset flagi przerwan
	TACCR0 = CLOCK_A_HZ * 2;	// Licznik liczy do tego losowego czasu
	TACCTL0 |= CCIE;		// Odblokowanie przerwan Timer_A0
	CLOCK_A_HZ = 16384;
	CLOCK_B_HZ = 32768;
}


int main(void)
{
	initSystem();
	initTimers();
	getRandomData();

	while(true)
	{
		__enable_interrupt();
		__bis_SR_register(LPM3_bits + GIE);

		if(oscilateFlag)
		{
			eliminateOscilations();

			P1IE |= BIT7;

			oscilateFlag = false;
		}
	}
}

void eliminateOscilations()
{
	long long int i;
	for(i = 20000; i > 0; i--);

	volatile int flag = 1;

	while(flag)
	{
		if(P1IN & BIT7) {flag = 0;}
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{

	else if( !(P1IN & BIT7) && (P1_INTERR_MODE == 3))
	{
		resetVariables();

		TIMER_A_INTERR_MODE = WAIT_FOR_ACTIVATION;

		resetTimer(&TBCCTL0);
		P2OUT = 0xFF;

		P1_INTERR_MODE = SWITCH_ON;

		P1IE &= ~BIT7; //zablokuj przerwania

		oscilateFlag = true; // eliminate oscilations

		setTimer_A2();

		LPM3_EXIT; // chcemy wyeliminowac drgania stykow zeby nie przeszedl od razu do pierwszego if w tym przerwaniu
	}

	P1IFG &= 0x00;			// Wyzerowanie flag przerwan P1
}

// Timer A0 odlicza poczatkowy losowy czas
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	if(TIMER_A_INTERR_MODE == RANDOMTIME)		// TIMER A0
	{
		TACCTL0 &= ~CCIE;	// Blokuje przerwania Timer_A0
		prepareTimerA_ForCount();
		setStopButton();
	}
	else if(TIMER_A_INTERR_MODE == COUNT)							// TIMER A1
	{
		if(++miliseconds > 99)
		{
			miliseconds = 0;
			if(++seconds >= 10)
			{
				resetTimer(&TACCTL0); // zeby nie liczyl w nieskonczonosc jak ktos odszedl od miernika
				P1IE  |= BIT7;		// Odblokowanie przerwan na P1.7
				P1_INTERR_MODE = 3;
			}
		}

		TACCTL1 &= ~CCIFG;	// Resetuje flage przewania
	}
	else if(TIMER_A_INTERR_MODE == WAIT_FOR_ACTIVATION)	// TIMER A2
	{
		P4OUT = BUTTONS[num];
		if(++num > 7)
			num = 0;
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
			refreshDisplay(miliseconds);
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
			P3OUT &= 0x7F;						// Dodanie kropki po liczbie sekund
			break;
		}
		case 3:		// Odswiez dziesiatki sekund
		{
			refreshDisplay((seconds / 10) % 10); // Jesli seconds = 48 dostajemy num = 4
			refresh_id = 0;
			break;
		}
	}
}
