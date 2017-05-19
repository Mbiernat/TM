#define b 2
#if b==2

#include <msp430.h>

#define maxDEC 5 //ograniczenie dla dziesiatek
#define maxI 9 //ograniczenie dla jednosci

/*
 * P1.0 - START
 * P1.1 - RESET
 * P2 - kable do rozbrajania (3 - poprawny)
 * P3 - wybieranie pozycji na wyswietlaczu
 * P4 - wartosc ladowana do wyswietlacza
 * P5 - wartosc sekund ladowana z HEX do licznika (JED i DEC)
 * P6 - wartosc minut ladowana z HEX do licznika (JED i DEC)
 */

unsigned int SEC=32768; //czestotliwosc dla licznika timer0 (sekundy)
int RFSH=100; //czestotliwosc dla odswiezania wyswietlacza
int time; //obecny czas

int timeTab[4]; //mm:ss -> tab[3]=m, tab[2]=m, tab[1]=s, tab[0]=s
int timeTabMask[6]; //tablica mm:ss z maskami dla wyswietlacza
int explosionDispMask[]={0xF7,0xF7,0xF1,0xF0,0xF0,0xF8};
int diffuseDispMask[]={0x7F,0x7F,0x7F,0x7F,0xFF,0xFF};

int defuse=1;

int state=0; //0 - init ,1 - liczenie, 2 - eksplozja, 3 - rozbrojenie

int pos=0; //obecna pozycja na wyswietlaczu

void countTimeTab(); //wyliczanie tablicy mm:ss z obecnego czasu w sek
void getTime(); //pobieranie czasu z nasatwników hex
void countTimeMasks(); //wyliczanie masek dla wyswietlacza

int main(void){
    WDTCTL = WDTPW | WDTHOLD; //wylaczenie watchdoga

    P3DIR|=0xFF; //port 3 - wyjscie
    P4DIR|=0xFF; //port 4 - wyjscie

    P1IE=0x01; //odblokowane przerwania na pin 1 port 1
    P1IES=0x03; //przerwania na pin 1 i 2 na port 1 na zboczu opadajacym

    P2IES=0x0F; //przerwania w port 2 na zboczu opadajacym

    TACTL|=ID_0+TASSEL_1+MC_2+TAIE; //prescaler = 1, zegar ACLK, licznik wylaczony, odblokowane przerwania

    TACCTL0&=~CAP; //tryb timer0 comapre
    TACCTL1&=~CAP; //tryb timer1 compare

    TACCTL1|=CCIE; //przerwania dla timer1

    //TACCTL2|=CAP+CCIE;

    TACCR1=RFSH-1; //ladowanie wartosci RFSH-1 do rejestru licznika 1 rfsh

    TAR=0; //zerowanie wartosci rejestru licznika A

    __bis_SR_register(GIE);

    for(;;){
        if(time<0) {
        	P2IFG=~(1<<defuse);
        	}

        if(state==0) getTime();

        countTimeTab(); //wyliczenie tablicy mm:ss z obecnego czasu
        countTimeMasks(); //wyliczenie tablicy mm:ss z maskami dla wyswietlacza

        __bis_SR_register(LPM0_bits); //uspienie + wlaczenie globalnej maski przerwan
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void START_RESET(void){
    if((P1IFG & 0x01) && (P1IE & 0x01)){
        state=1;
        P1IE&=~(0x01); //zablokowanie przerwañ na przycisku P1.0
        P2IE|=0x0F; //odblokowanie przerwan na porcie 2
        TACCR0=TAR+SEC; //ustawienie nowej wartosci w rejestrze licznika
        TACCTL0|=CCIE; //odblokowanie przerwan na liczniku
        TACCTL0&=~CCIFG; //wyczyszczenie flagi przerwan dla licznika
        P2IFG=0x00;

        //TACCTL2|=CCI;
    }
    if((P1IFG & 0x02) && (P1IE & 0x02)){
        state=0;
        P1IE&=~0x02;
        P1IE|=0x01;
    }
    P1IFG=0x00;
}

#pragma vector=PORT2_VECTOR
__interrupt void DEFUSE(void){
    if(P2IFG & (1<<defuse)) state=3;
    else state=2;

    TACCTL0&=~CCIE;
    time=0;
    P2IE&=~0x0F;
    P1IE|=0x02;
    P2IFG=0x00;
    P1IFG=0x00;
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TIME(void){
    --time; //dekrementacja czasu

    TACCR0+=SEC; //nowa wartosc rejestru licznika
    TACCTL0&=~CCIFG; //wyczyszczenie flagi przerwania na kanale 0

    __bic_SR_register_on_exit(LPM0_bits); //wlaczenie procesora po zakonczeniu przerwania
}

#pragma vector=TIMERA1_VECTOR
__interrupt void REFRESH(void){
    switch (TAIV){
    case 0x02:
        P3OUT=0xFF; //wylaczenie pozycji
        P4OUT=timeTabMask[pos]; //uzycie odpowiedniej maski z czasem dla danej pozycji
        P3OUT&=~(1<<pos); //wlaczenie danej pozycji
        pos=(pos + 1) % 6;
        TACCR1+=RFSH;
        TACCTL1&=~CCIFG;
        break;
    /*case TAIV_TACCR2:
        defuse=1<<(TACCR2 % 3);
        TACCTL2&=~(CCIE+CCI);
    break;*/
    case 0xA0:
        TACTL&=~TAIFG;
        break;
    }

    __bic_SR_register_on_exit(LPM0_bits);
}

void countTimeTab(){
    int sec,min;
    if(time<0) time=0;

    min=time/60;
    sec=time%60;

    timeTab[0]=sec%10;
    timeTab[1]=sec/10;

　
    timeTab[2]=min%10;
    timeTab[3]=min/10;
}

void getTime(){
    int minDEC,minI,secDEC,secI; //minDEC, secDEC - dziesietne wartosci minut, sekund; minI, secI - jednosci minut, sekund; min,sec - minuty, sekundy
    minDEC=(P6IN & 0xF0)/(0x0F); //wartosc na pinach 4..7 portu 6 - wart. dec. minut
    minI=P6IN & 0x0F; //wartosc na pinach 0..3 portu 6 - wart. jedn. minut
    secDEC=(P5IN & 0xF0)/(0x0F); //wartosc na pinach 4..7 portu 5 - wart. dec. sek.
    secI=P5IN & 0x0F; //wartosc na pinach 0..3 portu 5 - wart. jedn. sek.

    //sprawdzenie ograniczen
    if(minDEC>maxDEC) minDEC=maxDEC;
    if(minI>maxI) minI=maxI;
    if(secDEC>maxDEC) secDEC=maxDEC;
    if(secI>maxI) secI=maxI;

    //zapis do tablicy mm:ss
    timeTab[3]=minDEC;
    timeTab[2]=minI;
    timeTab[1]=secDEC;
    timeTab[0]=secI;

    time=(minDEC*10+minI)*60+(secDEC*10+secI); //zapis do aktualnego czasu (w sek.)
}

void countTimeMasks(){
    //obliczenie masek dla wejscia danych wyswietlacza w formie mm.ss
    int i;
    switch (state){
    case 0:
        for(i=0;i<4;i++) if(i!=2) timeTabMask[i]=timeTab[i] + 0xF0;
        timeTabMask[2]=timeTab[2] + 0x70;
        timeTabMask[4]=0xFF;
        timeTabMask[5]=0xFF;
        break;

    case 1:
        for(i=0;i<4;i++)
        if(i!=2) timeTabMask[i]=timeTab[i] + 0xF0;
        timeTabMask[2]=timeTab[2] + 0x70;
        timeTabMask[4]=0xFF;
        timeTabMask[5]=0xFF;
        break;

    case 2:
        for(i=0;i<6;i++) timeTabMask[i]=explosionDispMask[i];
        break;

    case 3:
        for(i=0;i<6;i++) timeTabMask[i]=diffuseDispMask[i];
    }
}

#endif
