#ifndef HEADERS_LAB4_H_
#define HEADERS_LAB4_H_

#include <msp430.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//-----------------------------------------------------------------
void getRandomData();

void initTimers();

void initSystem();

void prepareCounterB_ForRefresh();

void setStopButton();

void setTimer_A0();

void setTimer_A2();

void resetVariables();

void resetTimer_B();

void resetTimer(volatile unsigned int *timer);

void eliminateOscilations();

void refreshDisplay(int segment);

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

//-----------------------------------------------------------------

#endif /* HEADERS_LAB4_H_ */
