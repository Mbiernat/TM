//przeslij przez usart wartosc z dwoch nastawnikow

#include <msp430.h> 

void initUart();
void initSystem();

void initUart()
{
    // zewnetrzny kwarc zeby dzialal bardzo szybko
    BCSCTL1 &= ~XT2OFF; // XT2 on
    BCSCTL2  = SELS; // wybiera XT2CLK bazujący na zewnętrznym kwarcu i daje na SMCLK source XT2CLK

    //piny 3.4 i  3.5 jako we/wy TX/RX
    P3SEL |= BIT4 + BIT5; // druga funkcja pinow

    U0CTL = SWRST | CHAR | PENA | PEV | SPB; // USART control register, 271 manual
    U0TCTL |= SSEL1; // transmit control register select SMCLK skonfigurowany powyzej
    ME1 = UTXE0 + URXE0; // mozna odbierac i wysylac dane, module enable register
    U0CTL &= ~SWRST;

    IE1 |= URXIE0; // przerwania od odbierania
    IE1 &= ~UTXIE0;
}

void initSystem()
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    P1DIR = 0x00; // input
    P1IES = 0xFF; // from high to low
    P1IE = BIT0; // first button

    // nastawniki
    P2DIR = 0x00; //input (2 nastawniki heksadecymalne)
}

void main(void)
{
    initSystem();
    initUart();

    while(1)
    {
        __enable_interrupt();
        __bis_SR_register(LPM1_bits + GIE);
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    TXBUF0 = P2IN; // gdyby wiele danych to odblokowane przerwania dla wysylania z uarta i weszloby do obslugi przerwania
    // zwiazanego z buforem cyklicznym (tutaj brak takiej opcji)

    P1IFG &= ~BIT0;
}
