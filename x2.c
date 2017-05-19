
#include <msp430.h>
#include <stdint.h>
#include "include/system.h"
#include "include/watchdog.h"

extern volatile uint16_t timer_10ms_ticks;
volatile uint8_t display_data[8] = {0xF0,0xF0,0x70,0xF0,0x70,0xF0,0xF0,0xF0};
volatile uint8_t position;
volatile uint8_t setseconds;
volatile uint8_t seconds;
volatile uint8_t minutues;
volatile uint8_t counter;

　
int main(void)
{
  watchdog_disable();
  system_init();
  
  
   __enable_interrupt();
   //  __low_power_mod_1();
   //  __bis_SR_register(GIE);
   //DISPLAY_CONTROL = 0x00;
   while(1){
	   display_data[0] = (setseconds%10) | 0xF0;
	   display_data[1] = (setseconds/10)| 0xF0;

	   if(seconds>=60){
		   seconds = 0;
		   minutues++;
		   display_data[4] = (minutues%10) | 0x70;
		   display_data[6] = (minutues/10)| 0xF0;
	   }
	   if(setseconds>=100){
		   setseconds -=100;
		   seconds++;
		   display_data[2] = (seconds%10) | 0x70;
		   display_data[3] = (seconds/10)| 0xF0;
	   }

   }
}

　
　
// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{

    DISPLAY_DATA=display_data[position];
    DISPLAY_CONTROL&=~(1<<position); //wlaczenie danej pozycji
    position=(position + 1) & 0x07;
    counter++;
    if(counter>=16){
    	setseconds++;
    	counter = 0;
    }
    TACCTL0&=~CCIFG;
}

// Timer_A3 Interrupt Vector (TAIV) handler
#pragma vector=TIMERA1_VECTOR
__interrupt void Timer_A1(void)
{
	  DISPLAY_CONTROL=0xFF; //wylaczenie pozycji
	  TACCTL1&=~CCIFG;
}
