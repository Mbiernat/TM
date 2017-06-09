#include <msp430.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "headers/lab4.h"

enum TIMER_A_INTERR_MODE {RANDOMTIME, COUNT, ELIMINATE_OSCILATIONS};
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

	P2OUT = 0x00;			// Gaszenie wszystkich segmentów wyswietlacza
	P3OUT = 0x00;			//
	P4OUT = 0x00;			// Gaszenie diod
	int i;
	for(i=0; i<8; i++)
		displayBuffer[i]=0x00;

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
	int i;
	for(i=0; i<8; i++)
			displayBuffer[i]=0x00;

	seconds = refresh_id = word_id = numOfButton = miliseconds = 0;
	P4OUT = P1IFG = 0x00;
	P1IE = 0x80;

}

//-----------------------------------------------------------------

int main(void)
{
	initSystem();
	initTimers();
	getRandomData();

	while(true)
	{
		__enable_interrupt();
		__bis_SR_register(LPM3_bits + GIE);

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

	if( !(P1IN & BIT7) & (P1_INTERR_MODE == 0))		// Poczatkowe uruchamianie licznika - od tego momentu liczenie losowych sekund
	{
		getRandomData();
		setTimer_A0();
		for(k=0;k<4;k++)	displayBuffer[k] = displayedSymb[0];
		displayBuffer[2] |= 0x80;
		P1IE  &= ~BIT7;		// Zablokowanie przerwan na P1.7

		prepareCounterB_ForRefresh();

		word_id = 0;

		P1_INTERR_MODE = STOP_TIMER;
	}
	else if(P1_INTERR_MODE == 1)
	{
		resetTimer(&TACCTL0);

		P1IE  |= BIT7;		// Odblokowanie przerwan na P1.7

		P1_INTERR_MODE = 3;

		if(seconds < 2)
		{
			displayBuffer[4] = displayedSymb[13];
			displayBuffer[5] = displayedSymb[12];
			displayBuffer[6] = displayedSymb[12];
			displayBuffer[7] = displayedSymb[11];
		}

		else
			{
			displayBuffer[4] = displayedSymb[10];
			displayBuffer[5] = displayedSymb[13];
			displayBuffer[6] = displayedSymb[15];
			displayBuffer[7] = displayedSymb[14];
			}
	}
	else if( !(P1IN & BIT7) && (P1_INTERR_MODE == 3))
	{
		resetVariables();

		TIMER_A_INTERR_MODE = ELIMINATE_OSCILATIONS;

		resetTimer(&TBCCTL0);
		P2OUT = 0x00;

		P1_INTERR_MODE = SWITCH_ON;

		P1IE &= ~BIT7; 		//zablokuj przerwania

		oscilateFlag = true; 	// eliminate oscilations

		setTimer_A2();

		word_id = 0;

		LPM3_EXIT; 		// chcemy wyeliminowac drgania stykow zeby nie przeszedl od razu do pierwszego if w tym przerwaniu
	}

	P1IFG &= 0x00;			// Wyzerowanie flag przerwan P1
}

void setTimer_A0()
{
	TACCTL0 &= ~CCIFG; 			// reset flagi przerwan
	TIMER_A_INTERR_MODE = RANDOMTIME;
	TACCR0 = CLOCK_A_HZ * 2;		// Licznik liczy do tego losowego czasu
	TACCTL0 |= CCIE;				// Odblokowanie przerwan Timer_A0
}

void setTimer_A2()
{
	TACCTL0 &= ~CCIFG;
	TIMER_A_INTERR_MODE = ELIMINATE_OSCILATIONS;
	TACCR0 = CLOCK_A_HZ/ 1000;			// Licznik liczy do tego losowego czasu
	TACCTL0 |= CCIE;				// Odblokowanie przerwan Timer_A0
}

void prepareCounterB_ForRefresh()
{
	// Timer B do wyswietlania - odswiezanie diod
	TBCCR0 = CLOCK_B_HZ / 880;			// Licznik liczy co 1/440 sekundy
	TBCCTL0 |= CCIE;				// Odblokowanie przerwan Timer_B0
}

void prepareTimerA_ForCount()
{
	// Timer A0 do liczenia
	TIMER_A_INTERR_MODE = COUNT;			// Zmiana roli dla Timer_A0
	TACCR0 = CLOCK_A_HZ / 100;			// Licznik liczy co 1/100 sekundy
	TACCTL0 |= CCIE;				// Odblokowanie przerwan Timer_A1
	// Odblokowanie przycisku do zatrzymywania
}

void setStopButton()
{
	// Odblokowanie przycisku do zatrzymywania stopera
	P1IFG = 0;
	P1IE = P4OUT = BUTTONS[numOfButton];	// Odblokowanie przerwania i Zaswiecenie diody odpowiadajacej przyciskowi
}

// Timer A0 odlicza poczatkowy losowy czas
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	if(TIMER_A_INTERR_MODE == RANDOMTIME)	// TIMER A0
	{
		TACCTL0 &= ~CCIE;		// Blokuje przerwania Timer_A0
		prepareTimerA_ForCount();
		setStopButton();
	}
	else if(TIMER_A_INTERR_MODE == COUNT)	// TIMER A1
	{
		miliseconds++;
		displayBuffer[0] = displayedSymb[miliseconds % 10];
		displayBuffer[1] = displayedSymb[miliseconds / 10];
		displayBuffer[3] = displayedSymb[0];
		if(miliseconds > 99)
		{
			seconds++;
			displayBuffer[2] = displayedSymb[seconds % 10] + 0x80;	//!!!!!!!!!!!!!!!!!!!!!!!
			miliseconds = 0;
			if(seconds >= 10)
			{
				displayBuffer[0] = displayedSymb[0];
				displayBuffer[1] = displayedSymb[0];
				displayBuffer[2] = displayedSymb[0] + 0x80;
				displayBuffer[3] = displayedSymb[1];
				displayBuffer[4] = displayedSymb[15];
				displayBuffer[5] = displayedSymb[17];
				displayBuffer[6] = displayedSymb[16];
				displayBuffer[7] = displayedSymb[13];

				resetTimer(&TACCTL0); 	// zeby nie liczyl w nieskonczonosc jak ktos odszedl od miernika
				P1IE  |= BIT7;			// Odblokowanie przerwan na P1.7
				P1_INTERR_MODE = 3;
				word_id = 3;
			}

		}

		TACCTL1 &= ~CCIFG;	// Resetuje flage przewania
	}
	else if(TIMER_A_INTERR_MODE == ELIMINATE_OSCILATIONS)	// TIMER A3
	{
		oscilateFlag_prev = oscilateFlag;
		if(!(P1IN & BIT7))
		{
			oscilateFlag = true;
			if (oscilateFlag_prev == oscilateFlag)
			{
				P1IE |= BIT7;
				oscilateFlag = oscilateFlag_prev = false;
			}
		}
		else
			oscilateFlag = false;

		if(tick++ > 250)
		{
			P4OUT = BUTTONS[num];
			if(++num > 7)
				num = 0;
			tick = 0;
		}
	}

	TACCTL0 &= ~CCIFG;	// Resetuje flage przerwania
}

// Timer B0 do odswiezania liczb na ekranie
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B0 (void)
{
	P3OUT = 0x00;
	P2OUT = numOfDisplay[refresh_id];
	P3OUT = displayBuffer[refresh_id++];
	if (refresh_id == 8)
		refresh_id = 0;

}


void getRandomData()
{
	randomTime = (1 + (rand() % 5)) * CLOCK_A_HZ;	// otrzymamy czas w sekundach, Trzeba pomnozyc czas w sekundach zeby otrzymac w Hz dla zegara
	numOfButton = rand() % 7;			// Przyciski do stopowania od 0 do 6
}

void resetTimer(volatile unsigned int *timer)
{
	// Zerowanie timera B0
	*timer = OUTMOD_5;	// Resetuje Timer_B0
	*timer &= ~CCIE;	// Blokuje przerwania Timer_B0
	*timer &= ~CCIFG;	// Resetuje flage przewania
}

