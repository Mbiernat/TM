#include <msp430.h> 

void initUart();
void initSystem();
void displayValue();

char receivedData;

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

    P4DIR = 0xFF;

    P3DIR |= BIT4; // do wysylania

    P4OUT = 0x00;
}

void displayValue()
{
    P4OUT = receivedData;
}

void main(void)
{
    initSystem();
    initUart();

    while(1)
    {
        __bis_SR_register(LPM1 + GIE);

        displayValue();
    }
}

#pragma vector = USART0RX_VECTOR
__interrupt void receiveData(void)
{
    receivedData = U0RXBUF;

    LPM1_EXIT;
}
