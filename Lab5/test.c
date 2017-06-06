
#include <msp430.h>

void initUart();
void initSystem();
void displayValue();

char volatile receivedData;
const short int refresh = 64; //  32678Hz

void initUart()
{
//	while(!(UTXE1 & UTXIFG1));
//    // zewnetrzny kwarc zeby dzialal bardzo szybko
//    // BCSCTL1 &= ~XT2OFF; // XT2 on
//    //BCSCTL2  = SELS; // wybiera XT2CLK bazujący na zewnętrznym kwarcu i daje na SMCLK source XT2CLK
//
//	U0BR1 = 0x00;
//	U0BR0 = 0x03;
//	U0MCTL = 0x4A;
//
//    //piny 3.4 i  3.5 jako we/wy TX/RX
//    P3SEL |= 0x30; // druga funkcja pinow
//
//    U0CTL = SWRST | CHAR; // USART control register, 271 manual
//    U0TCTL |= SSEL0; // ACLK
//    ME1 = URXE0 | UTXE0; // mozna odbierac  module enable register
//    U0CTL &= ~SWRST;
//
//    IE1 |= URXIE0 | UTXIE0; // przerwania od odbierania

	UCTL0 |= SWRST;

    TACTL = MC_1 | ID_0 | TASSEL_1; //clock init ACLK, mode Up, no divider.
    BCSCTL1 &= ~XTS; //'slow mode' ACLK
    TACCR0=refresh;

    BCSCTL2 |= SELS;// Korzystamy z zewnetrznego kwarcu jako zrodla taktowania (XT2CLK: Optional high-frequency oscillator 450-kHz to
//  8-MHz range. )
    P3SEL |= 0x30; //ustawiamy pierwsza i druga nozke jako Rx i Tx // P3.4 UTXD0 oraz P3.5 URDX0
    ME1 |= UTXE0 + URXE0;// Wlaczamy modul do wysylania danych i odbierania w UART
    UCTL0 |= CHAR;//PEV + PENA + CHAR + SPB;// Ustawiamy bit parzystosci, dlugosc danych 8 bitow, i 2 bity stopu

// Ustawienia dla 115200Hz (dla UART: max 115200 hz)
    UTCTL0 |= SSEL1; //typical 1,048,576-Hz SMCLK. --> LPM1
    UBR00 = 0x40; // pierwszy dzielnik czestotliwosci
    UBR10 = 0x00; // drugi dzielnik
    UMCTL0 = 0x00;
    UCTL0 &= ~SWRST; // wylaczenie software reset

   TACCTL0 |= CCIE; //clock interrupts enabled
   IE1 |= URXIE0; //przerwania wlaczone dla odbierania/dla wysylania
   IE1 |= UTXIE0;
}

void initSystem()
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    P4DIR = 0xFF;
    P4OUT = 0x04;
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
    	TXBUF0 = '5';
    	//RXBUF0 = 0x02;
        __bis_SR_register(GIE);

        displayValue();
    }
}

#pragma vector = USART0RX_VECTOR
__interrupt void receiveData(void)
{
    receivedData = U0RXBUF;
	//U0RXBUF = U0TXBUF;

    LPM1_EXIT;
}

#pragma vector = USART0TX_VECTOR
__interrupt void sendData(void)
{
    receivedData = 0x08;

    LPM1_EXIT;
}
