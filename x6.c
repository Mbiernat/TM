#define a 2
#if a==1
#include <msp430.h>
/*
 * P1 przyciski (start i reset) 1-start, 2-reset, 3-timer out
 * P2 kable do rozbrajania
 * P3 pozycja na wyswietlaczu
 * P4 wartoœæ na wyswietlaczu
 * P5 hex sekundy
 * P6 hex minuty
 */
//
//********DEFINE*************
#define LICZBA_CYFR 4
#define REFRESH 100
#define COUNT 32768

　
//********ZMIENNE*************

int czas[LICZBA_CYFR]; // 0,1 s 2,3 m 4,5 h
int pos; //pozycja do wyswietlenia
int licznik; //sluzy w procedurze uaktualniania licznika zegara
int def_cable;
int pmask;
int decrement=0;
int in_reset_mode=1;

//*******STA£E****************
const int max[LICZBA_CYFR] = {9,5,9,9};
const int mask[LICZBA_CYFR] = {0xF0,0xF0,0x70,0xF0};

//wczytanie czasu z hex
void set_time()
{
    int i;
    czas[0]=P5IN & 0x0F;
    czas[1]=(P5IN & 0xF0)/(0x0F);
    czas[2]=P6IN & 0x0F;
    czas[3]=(P6IN & 0xF0)/(0x0F);

    for (i=0; i<LICZBA_CYFR; i++)
    {
        if (czas[i]>max[i]) czas[i]=max[i];
    }
    return;
}

void init_timer()
{
    /*Z LOSOWANIEM ROZBRAJANIA
    TACCTL2|=CAP+CCIE;

     */
    TACCTL0&=~CAP; //compare ccr0
    TACCTL1&=~CAP; //compare ccr1
    TACCTL0|=CCIE; //wl¹cz przerwania ccr0
    TACTL|=MC_2+ID_0+TASSEL_1+TAIE+TACLR; //continous, clock ACLK (32khz), przerwania

    return;
}

//wyœwietlanie
#pragma vector = TIMERA0_VECTOR
__interrupt void ISR_REFRESH (void) {

    P3OUT=0xFF; //nie wskazuj zadnej pozycji
    P4OUT=mask[pos]+czas[pos]; //wystaw dane
    P3OUT=~(1<<pos); //wskaz pozycje
    pos=(pos+1)%LICZBA_CYFR;
    TACCR0=TACCR0+REFRESH;
    TACCTL0&=~CCIFG;

    //if(in_reset_mode)
    	__bic_SR_register_on_exit(LPM0_bits);
}
//up³yw czasu
#pragma vector = TIMERA1_VECTOR
__interrupt void ISR_COUNT (void) {
    switch(TAIV)
    {
    case TAIV_TACCR1:
          czas[0]--;
		  decrement=1;
          TACCR1=TACCR1+COUNT;
          TACCTL1&=~CCIFG;

          __bic_SR_register_on_exit(LPM0_bits);
     break;

     /*Z LOSOWANIEM ROZBRAJANIA
     case TAIV_TACCR2:
         def_cable=1<<(TACCR2 % 3);
         TACCTL2&=~(CCIE+CCI);
     break;
     */
     default:
    	 TACTL&=~TAIFG;
     break;
    }
}

//przycisk start i reset
#pragma vector = PORT1_VECTOR
__interrupt void ISR_BUTTON (void) {

  if((P1IFG & BIT0) && (P1IE & BIT0))
  {
	  TACCR0=TAR+REFRESH-1; //zal¹duj ccr0
      //wcisniêto start
      P1IE=0x00; //wylacz przerwania przycisków
      P2IE=0x0F; //wl¹cz przerwania kabli
      licznik=LICZBA_CYFR-1;
      TACCR1=TAR+COUNT-1; //ladowanie licznika
      TACCTL1|=CCIE; //wl¹cz przerwania
	  in_reset_mode=0;
      /*Z LOSOWANIEM ROZBRAJANIA
      TACCTL2|=CCI;
       */
  }
  if ((P1IFG & BIT1) && (P1IE & BIT1))
  {   //wcisnieto reset
      /*Z LOSOWANIEM ROZBRAJANIA
            TACCTL2|=CCI;
      */
	  in_reset_mode=1;
	  set_time();
      P1IE=0x01; //wlacz przerwania przycisku start, wylacz reset
	  __bic_SR_register_on_exit(LPM0_bits);
  }
  P1IFG=0x00;
}

//kable
#pragma vector = PORT2_VECTOR
__interrupt void ISR_DEFUSE (void) {

    switch(P2IFG)
    {
    case BIT0:
        //rozbrojone
        TACCTL1&=~CCIE; //wyl¹cz przerwania ccr1
    break;

    case BIT1:
    case BIT2:
    case BIT3:
        //wybuch
        TACCTL1&=~CCIE; //wyl¹cz przerwania ccr1
        int i;
        for (i=0; i<LICZBA_CYFR; i++)
        {
            czas[i]=0;
        }
    break;
    }
    /*Z LOSOWANIEM ROZBRAJANIA
    switch(P2IFG)
      {
      case def_cable:
          //dozbrojone
          TACCTL1&=~CCIE; //wyl¹cz przerwania ccr1
      break;

      default:
          //wybuch
          TACCTL1&=~CCIE; //wyl¹cz przerwania ccr1
          int i;
          for (i=0; i<LICZBA_CYFR; i++)
          {
              czas[i]=0;
          }
      break;
      }

    */
	P1IFG=0x00;
    P1IE=0x02; //wlacz przerwanie reset
    P2IE=0x00; //wylacz przerwania kable
    P2IFG=0x00;
    TACCTL1&=~CCIFG;
}

　
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P3DIR=0x00FF; //wyjscie (pozycja na wyswietlaczu)
    P4DIR=0x00FF; //wyjscie (dane na wyswietlacz)

    P1IES=0x03;//zbocze opadaj¹ce
    P2IES=0x0F;//zbocze opadaj¹ce

    P3OUT=0xFF;
    P4OUT=0x00;

    P1IE=0x01;//w³¹czone przerwanie przycisku start

    init_timer(); //wlacz timer refresh

    //__bis_SR_register(LPM0_bits+GIE); //spij z w³¹czonymi przerwaniami

    for(;;)
    {
		if (decrement)
		{
			int i;
			for(i=0; i<licznik; i++)
			{
				if(czas[i]<0)
				{
					czas[i]=max[i];
				czas[i+1]--;
				}
				else break;
            }

			if (czas[licznik]==0) //jeœli ostatnia niezerowa cyfra sie zeruje
     		{
				if (licznik==0) //jeœli ta cyfra to 1 cyfra
				{
					P2IFG=BIT3; //detonator wybucha
					break;
				}
            licznik--;
            }
		}

		if (in_reset_mode==1)
		{
			set_time();
		}

    	 __bis_SR_register(GIE); //spij z wlaczonymi przerwaniami
   }

	return 0;
}
#endif
