#include  <msp430.h>
#include <time.h>

//-----------------------------------------------------------------
void getRandomData();

void initTimers();

void setStopwatch();

void setStopButton();

void disableStopwatch();


//-----------------------------------------------------------------
volatile char CLOCK_HZ;		// Do przechowania aktualnej częstości zegara

volatile char numOfButton; 	// Numer przycisku, który trzeba wcisnąć
unsigned char StopButton[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};	// Kody przycisku do zatrzymywania stopera   
	
volatile char randomTime;	// Losowy czas, po którym odpalamy stoper
	
volatile char seconds;		// Sekundy liczone przez stoper
volatile char miliseconds;	// Setne liczone przez stoper

volatile char refresh_id;	// Do wybierania który segment wyświetlacza odświeżamy
unsigned char numOfDisplay[] = {0xFE, 0xFD, 0xFB, 0xF7};
unsigned char displayedNum[] = {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9};
volatile char num;		// Zmienna pomocnicza

//-----------------------------------------------------------------

// Inicjalizacja programu
void system_init(void)
{
	WDTCTL = WDTPW + WDTHOLD; 	// Stop Watchdog timer
	
	// Input
	P1DIR &= 0x00; 			// Port1 - input - przyciski
	P6DIR &= 0x00;			// Port6 - input - nastawniki hex
	P1IE  |= BIT7;			// Odblokuj przerwania na P1.7
	P1IES |= 0xFF;			// Ustaw zglaszanie przerwania zboczem rosnacym

	// Output
	P2DIR |= 0xFF;			// Port2 - output - wyświetlacz dane
	P3DIR |= 0xFF;			// Port3 - output - wyświetlacz sterowanie
	P4DIR |= 0xFF;     		// Port4 - output - diody

	P2OUT |= 0xFF;			// Gaszenie wszystkich segmentów wyświetlacza
	P3OUT |= 0xFF;			// 
	P4OUT &= 0x00;			// Gaszenie diod
    
}
	
void initTimers()
{
	TACTL |= TASSEL_1 + ID_1 + MC_1;	// 16384Hz -> 1s
	TBCTL |= TASSEL_1 + ID_1 + MC_1;
	
	CLOCK_HZ = 16384;
	
	seconds = 0;
	miliseconds = 0;
}

//-----------------------------------------------------------------

int main(void)
{
	system_init();
	initTimers();
	getRandomData();

	
	while(1)
	{
		__enable_interrupt();
		__bis_SR_register(LPM3_bits + GIE);
		
		
		
	}
    
    return 0;
}


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if( !(P1IN & BIT7) & (P1IFG & BIT7) )	// Początkowe uruchamianie licznika - od tego momentu liczenie losowych sekund	
	{
		setTimer_A0();
		P1IFG &= ~BIT7;		// Wyzerowanie flagi przerwania P1.7
		P1IE  &= ~BIT7;		// Zablokowanie przerwań na P1.7
		// Odlicza losowy czas
	} 
	else 	// może trzeba jakiś jeszcze warunek
	{
		disableStopwatch();	// TO DO
		waitForActivation();	// TO DO
		
		P1IFG &= 0x80;		// Wyzerowanie flag przerwań przycisków P1.0 - P1.6
		P1IE  &= 0x80;		// Zablokowanie przerwań na P1.0 - P1.6
		
		P1IE  |= BIT7;		// Odblokowanie przerwań na P1.7
	}	
}

void setTimer_A0()
{
	// Wyzerować Timer_A
	TACCTL0 |= CCIE;	// Odblokowanie przerwań Timer_A0
	TAR &= 0x00;		// Reset TAR żeby zacząć liczenie od 0
	TACCR0 = randomTime;	// Licznik liczy do tego losowego czasu
	
}
	
void setStopwatch()
{
	// Timer A1 do liczenia
	TACCTL1 |= CCIE;		// Odblokowanie przerwań Timer_A1
	TAR &= 0x00;			// Reset TAR żeby zacząć liczenie od 0
	TACCR1 = CLOCK_HZ/100;		// Licznik liczy co 1/100 sekundy

	// Timer B do wyświetlania - odświeżanie diod
	TBCCTL0 |= CCIE;		// Odblokowanie przerwań Timer_B0
	TBR &= 0x00;			// Reset TBR żeby zacząć liczenie od 0
	TBCCR0 = CLOCK_HZ/50;		// Licznik liczy co 1/50 sekundy
	
	// Odblokowanie przycisku do zatrzymywania 
}

void setStopButton()
{
	// Odblokowanie przycisku do zatrzymywania stopera
	P1IE  = StopButton[numOfButton];	// Odblokowanie przerwania
	P4OUT = StopButton[numOfButton];	// Zaświecenie diody odpowiadającej przyciskowi
}

// Timer A0 odlicza początkowy losowy czas
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{
	TACCTL0 = OUTMOD_5;	// Resetuje Timer_A0
	TACCTL0 &= ~CCIE;	// Blokuje przerwania Timer_A0
	TACCTL0 &= ~CCIFG;	// Resetuje flagę przewania
	setStopwatch();
	setStopButton();
	
}
	
// Timer A1 do zliczania setnych i sekund
#pragma vector=TIMERA1_VECTOR
__interrupt void Timer_A1 (void)	
{
	miliseconds++;
	if(miliseconds > 99)
	{
		miliseconds = 0;
		seconds++;
	}
}

// Timer B0 do odświeżania liczb na ekranie
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B0 (void)	
{
	switch(refresh_id)
	{
		case 0:		// Odśwież setne sekundy
		{
			num = miliseconds % 10;			// Jeśli miliseconds = 48 dostajemy num = 8
			P3OUT = displayedNum[num];		// Wybór liczby do wyświetlenia na segment wyświetlacza
			P2OUT = numOfDisplay[refresh_id];	// Wybór segmentu wyświetlacza		
			refresh_id++;
			break;
		}
		case 1:		// Odśwież dziesiętne sekundy
		{
			num = (miliseconds/10) % 10;		// Jeśli miliseconds = 48 dostajemy num = 4
			P3OUT = displayedNum[num];
			P2OUT = numOfDisplay[refresh_id];	
			refresh_id++;
			break;
		}
		case 2:		// Odśwież sekundy
		{
			num = seconds % 10;			// Jeśli seconds = 48 dostajemy num = 8
			P3OUT = displayedNum[num];
			P2OUT = numOfDisplay[refresh_id];
			refresh_id++;
			break;
		}
		case 3:		// Odśwież dziesiątki sekund
		{
			num = (seconds/10) % 10;		// Jeśli seconds = 48 dostajemy num = 4
			P3OUT = displayedNum[num];
			P2OUT = numOfDisplay[refresh_id];
			refresh_id = 0;
			break;
		}
		default:	{ refresh_id = 0;	break; }	
	}
}

void getRandomData()
{
	srand(time(NULL));
	randomTime = (rand() % 300)/100.0;	// otrzymamy czas w sekundach
	randomTime *= CLOCK_HZ; 		// Trzeba pomnożyć czas w sekundach żeby otrzymać w Hz dla zegara 
	numOfButton = rand() % 7;		// Przyciski do stopowania od 0 do 6
}

void disableStopwatch()
{
	// Zerowanie timera A1
 	TACCTL1 = OUTMOD_5;	// Resetuje Timer_A1
	TACCTL1 &= ~CCIE;	// Blokuje przerwania Timer_A1
	TACCTL1 &= ~CCIFG;	// Resetuje flagę przewania
	
	// Zerowanie timera B0
	TBCCTL0 = OUTMOD_5;	// Resetuje Timer_B0
	TBCCTL0 &= ~CCIE;	// Blokuje przerwania Timer_B0
	TBCCTL0 &= ~CCIFG;	// Resetuje flagę przewania
}
