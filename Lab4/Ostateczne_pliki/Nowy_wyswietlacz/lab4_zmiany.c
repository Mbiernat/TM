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
volatile int refresh_id;// Do wybierania który segment wyswietlacza odswiezamy
volatile int word_id;// Do wybierania !!!!
unsigned int displayBuffer[8];
unsigned const int numOfDisplay[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};						// Do wyboru segmentu wyswietlacza
unsigned const int displayedSymb[] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B, 0x00, 0x7B, 0x7E, 0x3D, 0x1F,  0x77, 0x1C, 0x67};	// Do wyswietlania konkretnego numeru //0-9 <none> g o d b a u p
volatile int num;		// Zmienna pomocnicza
volatile int k, tick = 0;

volatile int TIMER_A_INTERR_MODE;
volatile int P1_INTERR_MODE;

volatile bool oscilateFlag = false;
volatile bool oscilateFlag_prev = false;


//-----------------------------------------------------------------

#endif /* HEADERS_LAB4_H_ */

