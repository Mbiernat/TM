// dima.c


#include <msp430.h>

/*
 * main.c
 */

#define HOLD_TIME 500
#define MAX_LEVEL 5

unsigned char logo[] = {0xF8, 0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1};
unsigned const char degree[] = {10, 20, 35, 52, 72};
unsigned char progress_bar[] = {0xFF, 0xFF, 0xF8, 0xF8, 0xF8, 0xF8, 0xFF, 0xFF};
unsigned char *tab = logo;
unsigned char position = 0;
unsigned char flag = 0;
unsigned char flag_button = 0;
unsigned int licznik = HOLD_TIME/20;
unsigned int licznik1 = HOLD_TIME;
unsigned char level = 2;

　
void update_progress_bar(unsigned char* temp)
{
	int k = 2;
	for (; k<6-level; ++k) temp[k] = 0x7F;
	for (; k<6; ++k) temp[k] = 0xF8;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    P3DIR |= 0xFF;	//port3 jako wyjscie
    P4DIR |= 0xFF;	//port4 jako wyjscie
    P3OUT = 0xFF;
    P4OUT = 0xFF;

    CCR0 = 80; //a tak na poczatek
    CCR1 = degree[level];
    CCTL0 = CCIE;
    CCTL1 = CCIE;
    TACTL |= TASSEL_1 + ID_0 + MC_1;

    while(1)
    {
    	_BIS_SR(CPUOFF + GIE);

    	if (level == 255) level = 0;
    	else if (level == MAX_LEVEL) level = MAX_LEVEL - 1;

    	tab = progress_bar;
    	update_progress_bar(tab);
    	licznik1 = HOLD_TIME;
    	flag = 1;
    	flag_button = 1;
    };
}

#pragma vector=TIMERA0_VECTOR
__interrupt void timer_a(void)
{
	P3OUT &= ~(1<<position);
	P4OUT = tab[position];

	position = position>7?0:position+1;
	if (flag)
	{
		CCR1 = degree[level];
		flag = 0;
	}
	if (!(P1IN & BIT0))
	{
		--licznik;
		if (!licznik) { LPM0_EXIT; --level; licznik = HOLD_TIME;}
		return;
	}
	else if (!(P1IN & BIT2))
	{
		--licznik;
		if (!licznik) { LPM0_EXIT; ++level; licznik = HOLD_TIME;}
		return;
	}
	if (licznik != HOLD_TIME/20) licznik = HOLD_TIME/20;
	if (flag_button) --licznik1;
	if (!licznik1) {tab = logo; licznik1 = HOLD_TIME; flag_button = 0;}
}

#pragma vector=TIMERA1_VECTOR
__interrupt void dark(void)
{
	if (TAIV != TAIV_TACCR1) return;
	P3OUT = 0xFF;
}
