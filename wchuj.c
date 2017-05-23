#include <msp430.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//-----------------------------------------------------------------
void getRandomData();

void initTimers();

void setStopwatch();

void setStopButton();

void setTimer_A0();

void disableStopwatch();

void waitForActivation();

void setLED_refresh();

void resetVariables();

void resetTimer_B();

void waitForPopButton();

void eliminateOscilations();

//-----------------------------------------------------------------
volatile int CLOCK_A_HZ;		// Do przechowania aktualnej czestosci zegara
volatile long CLOCK_B_HZ;

volatile int numOfButton; 	// Numer przycisku, który trzeba wcisnac
unsigned const int StopButton[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};	// Kody przycisku do zatrzymywania stopera

volatile long randomTime;	// Losowy czas, po którym odpalamy stoper

volatile int seconds;		// Sekundy liczone przez stoper
volatile int miliseconds;	// Setne liczone przez stoper

volatile int refresh_id;	// Do wybierania który segment wyswietlacza odswiezamy
unsigned const int numOfDisplay[] = {0xFE, 0xFD, 0xFB, 0xF7};						// Do wyboru segmentu wyswietlacza
unsigned const int displayedNum[] = {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9};	// Do wyswietlania konkretnego numeru
unsigned const int displayedNum_dot[] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79};	// Do wyswietlania konkretnego numeru
volatile int num;		// Zmienna pomocnicza

volatile int TIMER_A_INTERR_MODE;
volatile int TIMER_B_INTERR_MODE;
volatile int P1_INTERR_MODE;

volatile bool oscilateFlag = false;

//-----------------------------------------------------------------

// Inicjalizacja programu
void system_init(void)
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
	TBCTL |= TASSEL_1 + ID_0 + MC_1;	//32768 Hz -> 1s

	CLOCK_A_HZ = 16384;
	CLOCK_B_HZ = 32768;

	seconds = 0;
	miliseconds = 0;
}

void resetVariables()
{
	numOfButton = 0;
	refresh_id  = 0;
	seconds 	= 0;
	miliseconds = 0;
	TIMER_A_INTERR_MODE = 0;
	TIMER_B_INTERR_MODE = 0;
	P4OUT = 0x00;
	P1IFG = 0x00;
	P1IE = 0x80;

}

//-----------------------------------------------------------------

int main(void)
{
	system_init();
	initTimers();
	getRandomData();

	volatile int flag;

	while(1)
	{
		__enable_interrupt();
		__bis_SR_register(LPM3_bits + GIE);

		if(oscilateFlag)
		{
			eliminateOscilations()
		}
	}
}

void eliminateOscilations()
{
	for(int i = 20000; i > 0; i--);

　
　
	waitForPopButton();
}

void waitForPopButton()
{
	volatile int flag = 1;

	while(flag)
	{
		if(!(P1IN & BIT7))
			flag = 0;
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if( !(P1IN & BIT7) & (P1_INTERR_MODE == 0))		// Poczatkowe uruchamianie licznika - od tego momentu liczenie losowych sekund
	{
		setTimer_A0();
		P1IFG &= 0x00;		// Wyzerowanie flag przerwan P1
		P1IE  &= ~BIT7;		// Zablokowanie przerwan na P1.7
		setLED_refresh();

		P1_INTERR_MODE = 1;
	}
	else if(P1_INTERR_MODE == 1)			// moze trzeba jakis jeszcze warunek
	{
		disableStopwatch();

	//------------------------------

	//	waitForActivation();

　
　
	//	P1IFG &= 0x80;		// Wyzerowanie flag przerwan przycisków P1.0 - P1.6
	//	P1IE  &= 0x80;		// Zablokowanie przerwan na P1.0 - P1.6

		P1IE  |= BIT7;		// Odblokowanie przerwan na P1.7
		P1_INTERR_MODE = 3;
	}
	else if( !(P1IN & BIT7) & (P1_INTERR_MODE == 3))
	{
		resetVariables();
		resetTimer_B();
		P2OUT = 0xFF;

		P1_INTERR_MODE = 0;
		//zablokuj przerwania
		P1IE &= ~BIT7;

		oscilateFlag = true;
	}
	P1IFG &= 0x00;			// Wyzerowanie flag przerwan P1
}

void setTimer_A0()
{
	TACTL |= TASSEL_1 + ID_1 + MC_1;	// 16384 Hz -> 1s
	TACCR0 = CLOCK_A_HZ* 2;				// Licznik liczy do tego losowego czasu
	TACCTL0 |= CCIE;					// Odblokowanie przerwan Timer_A0
}

void setLED_refresh()
{
	// Timer B do wyswietlania - odswiezanie diod
		TIMER_B_INTERR_MODE = 0;
		TBCTL |= TASSEL_1 + ID_0 + MC_1;	// 32768 Hz -> 1s
		TBCCR0 = CLOCK_B_HZ/440;			// Licznik liczy co 1/200 sekundy
		TBCCTL0 |= CCIE;					// Odblokowanie przerwan Timer_B0
}

void setStopwatch()
{
	// Timer A0 do liczenia
	TIMER_A_INTERR_MODE = 1;			// Zmiana roli dla Timer_A0
	TACTL |= TASSEL_1 + ID_1 + MC_1;	// 16384 Hz -> 1s
	TACCR0 = CLOCK_A_HZ/100;			// Licznik liczy co 1/100 sekundy
	TACCTL0 |= CCIE;					// Odblokowanie przerwan Timer_A1

　
　
	// Odblokowanie przycisku do zatrzymywania
}

void setStopButton()
{
	// Odblokowanie przycisku do zatrzymywania stopera
	P1IFG = 0;
	P1IE  = StopButton[numOfButton];	// Odblokowanie przerwania
	P4OUT = StopButton[numOfButton];	// Zaswiecenie diody odpowiadajacej przyciskowi
}

// Timer A0 odlicza poczatkowy losowy czas
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	if(TIMER_A_INTERR_MODE == 0)		// TIMER A0
	{
	//	TACCTL0 = OUTMOD_5;	// Resetuje Timer_A0
		TACCTL0 &= ~CCIE;	// Blokuje przerwania Timer_A0
		TACCTL0 &= ~CCIFG;	// Resetuje flage przewania
		setStopwatch();
		setStopButton();
	}
	else							// TIMER A1
	{
		miliseconds++;
		if(miliseconds > 99)
		{
			miliseconds = 0;
			seconds++;
		}
		TACCTL1 &= ~CCIFG;	// Resetuje flage przewania
	}

}

/*
// Timer A1 do zliczania setnych i sekund
#pragma vector=TIMERA1_VECTOR
__interrupt void Timer_A1 (void)
{

	miliseconds++;
	if(miliseconds > 99)
	{
		miliseconds = 0;
		P4OUT ^= BIT6;
		seconds++;
	}
	TACCTL1 &= ~CCIFG;	// Resetuje flage przewania
}		*/

// Timer B0 do odswiezania liczb na ekranie
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B0 (void)
{
	if(TIMER_B_INTERR_MODE == 0)
	{
		switch(refresh_id)
			{
				case 0:		// Odswiez setne sekundy
				{
					num = miliseconds % 10;				// Jesli miliseconds = 48 dostajemy num = 8
					P3OUT = displayedNum[num];			// Wybór liczby do wyswietlenia na segment wyswietlacza
					P2OUT = numOfDisplay[refresh_id];	// Wybór segmentu wyswietlacza
					refresh_id++;
					break;
				}
				case 1:		// Odswiez dziesietne sekundy
				{
					num = (miliseconds/10) % 10;		// Jesli miliseconds = 48 dostajemy num = 4
					P3OUT = displayedNum[num];
					P2OUT = numOfDisplay[refresh_id];
					refresh_id++;
					break;
				}
				case 2:		// Odswiez sekundy
				{
					num = seconds % 10;					// Jesli seconds = 48 dostajemy num = 8
					P3OUT = displayedNum[num];
					P3OUT &= 0x7F;						// Dodanie kropki po liczbie sekund
					P2OUT = numOfDisplay[refresh_id];
					refresh_id++;
					break;
				}
				case 3:		// Odswiez dziesiatki sekund
				{
					num = (seconds/10) % 10;				// Jesli seconds = 48 dostajemy num = 4
					P3OUT = displayedNum[num];
					P2OUT = numOfDisplay[refresh_id];
					refresh_id = 0;
					break;
				}
				default:	{ refresh_id = 0;	break; }
			}
	}
	else
	{
		P4OUT |= StopButton[num];
		num++;
		if(num > 7)
		{
			TBCCTL1 = OUTMOD_5;	// Resetuje Timer_B1
			TBCCTL1 &= ~CCIE;	// Blokuje przerwania Timer_B1
			TBCCTL1 &= ~CCIFG;	// Resetuje flage przewania
		}
		TBCCTL1 &= ~CCIFG;	// Resetuje flage przewania
	}

}

/*
// Timer B1 do odswiezania liczb na ekranie
#pragma vector=TIMERB1_VECTOR
__interrupt void Timer_B1 (void)
{
	P4OUT |= StopButton[num];
	num++;
	if(num > 7)
	{
		TBCCTL1 = OUTMOD_5;	// Resetuje Timer_B1
		TBCCTL1 &= ~CCIE;	// Blokuje przerwania Timer_B1
		TBCCTL1 &= ~CCIFG;	// Resetuje flage przewania
	}
	TBCCTL1 &= ~CCIFG;	// Resetuje flage przewania
}			*/

void waitForActivation()
{
	num = 0;
//	TIMER_B_INTERR_MODE = 1;
//	TBCCTL0 |= CCIE;					// Odblokowanie przerwan Timer_B0
//	TBCTL |= TASSEL_1 + ID_0 + MC_1;	//32768 Hz -> 1s
//	TBCCR0 = CLOCK_B_HZ/4;				// Licznik liczy co 1/4 sekundy
}

void getRandomData()
{
//	srand(time(NULL));
	randomTime = 1 + (rand() % 4);		// otrzymamy czas w sekundach
	randomTime *= CLOCK_A_HZ; 		// Trzeba pomnozyc czas w sekundach zeby otrzymac w Hz dla zegara
//	numOfButton = 1;
	numOfButton = rand() % 7;		// Przyciski do stopowania od 0 do 6
}

void disableStopwatch()
{
	// Zerowanie timera A0
 	TACCTL0 = OUTMOD_5;	// Resetuje Timer_A0
	TACCTL0 &= ~CCIE;	// Blokuje przerwania Timer_A0
	TACCTL0 &= ~CCIFG;	// Resetuje flage przewania

	// Zerowanie timera B0
//	TBCCTL0 = OUTMOD_5;	// Resetuje Timer_B0
//	TBCCTL0 &= ~CCIE;	// Blokuje przerwania Timer_B0
//	TBCCTL0 &= ~CCIFG;	// Resetuje flage przewania
}

void resetTimer_B()
{
	// Zerowanie timera B0
	TBCCTL0 = OUTMOD_5;	// Resetuje Timer_B0
	TBCCTL0 &= ~CCIE;	// Blokuje przerwania Timer_B0
	TBCCTL0 &= ~CCIFG;	// Resetuje flage przewania
}
