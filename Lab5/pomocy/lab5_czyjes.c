#include <msp430.h>
/*
 * P3.0 - sygna CTS z komputera
 */

//ZAKTUALIZOWAC PO KAZDEJ ZMIANIE W KODZIE!!!
#define ADDR_BEGIN 0x0248       //adres w RAM od ktorego zaczynamy wykonywanie programu
//-----------------------------------------------------wektory przerwan--------------------------------------------------------------
#define RESET_ADDR  0x479E      //0xFFFE
#define NMI_ADDR    0x4832      //0xFFFC
#define RX_ADDR     0x46B2      //0xFFF2
#define TX_ADDR     0x4826      //0xFFF0
#define TIMERA_ADDR 0x477C      //0xFFEC
#define TAIFG_ADDR  0x481A      //0xFFEA



//-------------------------------------------------------zmienne--------------------------------------------------------------------
//wiadomosci dla uzytkownika
char text1[]="Wcisnij 's'\n\r";
char text2[]="Wgraj plik\n\r";
char text3[]="Blad\n\r\n";
char text4[]="Gotowe\n\r\n";
char buffer[4];                 //bufor pomocniczy
char lineLength;
char flag1=0x00;                //flaga pomocnicza
char flag2=0x00;                //flaga stanu
char flag3=0x00;                //flaga pomocnicza

int address=ADDR_BEGIN;         //zmienne 'adresowe'
int address2=ADDR_BEGIN;

int* memory=0x00;               //zmienne 'pamieciowe'
int* memory2=0x00;
int checkSum=0;                 //suma kontrolna
int data;                       //zmienna pomocnicza
char i,j;                       //indeksy
char b;

int byteCounter=0;              //ilosc wcztanych bajtow

int getData(int address,int i);
char getByte(int word);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

  //-------------------------------------------------inicjalizowanie zmiennych----------------------------------------------------------
  address=ADDR_BEGIN;
  byteCounter=0;
  buffer[0]=0;
  buffer[1]=0;
  buffer[2]=0;
  buffer[3]=0;
  flag1=0x00;
  flag2=0x00;

  //------------------------------------------------ustawienie timera-----------------------------------------------------------------

  //zegar SMCLK, przerwania co 10ms, tryb COMPARE
  TACTL = ID_0 + TASSEL_2 + MC_1 + TAIE;
  TACCTL0 &= ~CAP;
  TACCR0 = 10000;
  TAR=0;

  //------------------------------------------------ustawienie portow-------------------------------------------------------------------
  P2DIR=0xFF;
  P2OUT=0x01;
  //-----------------------------------------------konfiguracja UART----------------------------------------------------------------------
  //zegar XT2IN 7.3728 jako SMCLK

   BCSCTL1 &= ~XT2OFF;
   BCSCTL2  = SELS;

  //piny 3.4 i  3.5 jako we/wy TX/RX
   P3SEL = 0x30;

   U0CTL = SWRST + CHAR ;
   U0TCTL = 0x20;
   U0BR0 = 0x00;
   U0BR1 = 0x03;
   U0MCTL = 0x00;
   ME1 = UTXE0 + URXE0;
   U0CTL &= ~SWRST;

   IE1 |= URXIE0;
   IE1 &= ~UTXIE0;

   _BIS_SR(GIE);         // uspienie i odblokowanie przerwan

   for(;;){
	   _BIS_SR(LPM0_bits);           // uspienie procesora

	   P2OUT=flag2;
	          //------------------------------------------------------STAN 0---------------------------------------------------------------
	          //wyswietl wiadomosc sartowa i przejdz do stanu oczekiwania na plik
	          if(flag2==0x00){
	              IE1 |= UTXIE0;
	              i=0;
	              while(i<12){
	                  if(b  && (P3IN & BIT0)) {
	                	  b=0;
	                      U0TXBUF=text2[i];
	                      IE1 |= UTXIE0;
	                      i++;
	                  }
	              }
	              IE1 &= ~UTXIE0;

	              flag2=0x01;
	          }

	          //------------------------------------------------------STAN 1---------------------------------------------------------------
	          //wyluskiwanie pliku z RAMU i uporzadkowanie go
	          if(address2!=ADDR_BEGIN && flag2==1){
	             //zablokowanie przerwan dla transmisji
	             IE1 &= ~URXIE0;
	             IE1 &= ~UTXIE0;
	             //ustawienie zmiennych adresowych w stan poczatkowy
	             address2=ADDR_BEGIN;
	             address=ADDR_BEGIN;
	             //zresetowanie flag (oprocz flag2)
	             flag1=0;
	             byteCounter=0;
	             flag3=0;

	             //petla czytajaca i porzadkujaca plik
	             while(flag2==1){
	                 checkSum=0;
	                 buffer[0]=0; buffer[1]=0; buffer[2]=0; buffer[3]=0;

	                 //pierwszy bajt - dlugosc linii
	                 data=getData(address,flag1);
	                 lineLength=getByte(data);
	                 checkSum+=lineLength;

	                 //drugi bajt - bardziej znaczacy bajt adresu poczatka linii
	                 address+=2;
	                 data=getData(address,flag1);
	                 buffer[2]=getByte(data);
	                 checkSum+=buffer[2];

	                 //trzeci bajt - mnie znaczacy bajt adresu
	                 address+=2;
	                 data=getData(address,flag1);
	                 buffer[3]=getByte(data);
	                 checkSum+=buffer[3];

	                 //jesli adresem poczatka linii jest adres wektora przerwan to zapisz go
	                 if(buffer[2]==0xFF){
	                     memory=(int*)address2;
	                     *memory=((buffer[2] << 8) + buffer[3]);
	                     byteCounter+=2;
	                     address2+=2;
	                 }

	                 //czwarty bajt - identyfikator linii (0 - linia standardowa, 1 - ostatnia linia w pliku)
	                 address+=2;
	                 data=getData(address,flag1);
	                 buffer[0]=getByte(data);
	                 checkSum+=buffer[0];

	                 //jesli zostala wczytana ostatnia linia pliku - ustaw flag2=2
	                 if(buffer[0]==0x01) {
	                     P2OUT=0x01;
	                     flag2=2;
	                 }

	                 //petla wczytujaca dane z linii
	                 //operuje tutaj dwoma wskaznikami adresu, address1 jest adresem pliku wczytanego bezposrednio do RAMU
	                 //address2 - adres miejsca w pamieci gdzie znajduje sie juz przetlumaczony kod
	                 address+=2;
	                 for(i=lineLength;i>0;i--){
	                     //pobranie bajtu i zapisanie go do bufora podrecznego oraz do pamieci
	                     data=getData(address,flag1);
	                     buffer[flag3]=getByte(data);
	                     checkSum+=buffer[flag3];
	                     memory=(int*)address2;
	                     *memory=((buffer[0] << 8) + buffer[1]);

	                     //jesli zostaly wczytane dwa bajty to przejdz do nastepnego adresu pliku wlasciwego
	                     if(flag3 == 1)
	                         address2+=2;
	                     flag3=(flag3 + 1) & 0x01; //flag3 - ilosc wczytanych bajtow w buforze podrecznym

	                     //zwieksz licznik bajtow oraz adres pliku czytanego
	                     byteCounter++;
	                     address+=2;
	                 }

	                 //obliczanie sumy kontrolnej
	                 checkSum = checkSum & 0x00FF;
	                 checkSum = 0x100 - checkSum;

	                 //wczytanie sumy kontrolnej
	                 data=getData(address,flag1);
	                 buffer[0]=getByte(data);

	                 //sprawdzenie sumy kontrolnej, jesli jest niepoprawna ustaw flag2=3
	                 if(checkSum!=buffer[0]){
	                     flag2=3;
	                     P2OUT=0x03;
	                 }

	                 //ustaw nowy adres w zaleznosci od wspolczynnika przesuniecia wiersza
	                 if(flag1==0) address+=4;
	                 else address+=6;

	                 //flag1 jest wspolczynnikiem przesuniecia wiersza: 0 - wiersz bez przesuniecia, 1 - wiersz przesuniety o jeden bajt w lewo
	                 flag1=(flag1 + 1) & 0x01;
	             }
	          }

	          //------------------------------------------------------STAN 2---------------------------------------------------------------
	          //plik zostal poprawnie wczytany, wyswietl wiadomosc i przejdz do stanu 4
	          if(flag2==0x02){
	              P2OUT=0x01;
	              IE1 |= UTXIE0;
	              i=0;
	              while(i<13){
	                  if(b && (P3IN & BIT0)) {
	                	  b=0;
	                      U0TXBUF=text1[i];
	                      IE1 |= UTXIE0;
	                      i++;
	                  }
	              }
	              IE1 &= ~UTXIE0;
	              flag2=4;
	          }

	          //------------------------------------------------------STAN 3---------------------------------------------------------------
	          //plik zostal zle wczytany, wyswietl wiadmosc, zresetuj zmienne, odblokuj przerwania na RX i przejdz do stanu 0
	          if(flag2==0x03){
	              IE1 |= UTXIE0;
	              i=0;
	              while(i<7){
	                  if(b  && (P3IN & BIT0)) {
	                	  b=0;
	                      U0TXBUF=text3[i];
	                      IE1 |= UTXIE0;
	                      i++;
	                  }
	              }
	              IE1 &= ~UTXIE0;

	              address=ADDR_BEGIN;
	              address2=ADDR_BEGIN;
	              byteCounter=0;
	              buffer[0]=0;
	              buffer[1]=0;
	              buffer[2]=0;
	              buffer[3]=0;
	              flag1=0x00;
	              flag2=0x00;
	              IE1 |= URXIE0;
	          }

	          //------------------------------------------------------STAN 4---------------------------------------------------------------
	          //oczekiwanie na wcisniecie polecenie od uzytkownika, wgranie wektora przerwan i uruchomienie programu z RAMu
	          if(flag2==0x04){
	              //oczekiwanie na waidomosc
	              IE1 |= URXIE0;
	              flag1=0x00;
	              buffer[0]=0x00;
	              address=ADDR_BEGIN+byteCounter;

	              while(buffer[0]!='s'){
	                  address=ADDR_BEGIN+byteCounter;
	              }
	              IE1 &= ~URXIE0;

	              //wyczysc segment pamieci odpowiadajacy za wektor przerwan
	              __bic_SR_register(GIE);
	              memory=(int*)0xFE00;
	              FCTL2=FWKEY+FSSEL_1+FN0;
	              FCTL3=FWKEY;
	              FCTL1=FWKEY+ERASE;
	              *memory=0;
	              FCTL1=FWKEY;
	              FCTL3=FWKEY+LOCK;

	              //adres poczatku wektora przerwan wczytanego z pliku
	              address=ADDR_BEGIN+byteCounter-64;

	              //petla wczytujaca nowy wektor przerwan
	              for(i=16;i>0;i--){
	                  memory=(int*)address;
	                  memory2=(int*)(*memory);

	                  //nie wczytujemy wszystkich wektorow przerwan pobranych z pliku
	                  //wektory RESET, NMI(OSCILLATOR FAULT), TIMERA, TAIFG, RX, TX pozostawiamy bez zmian
	                  if(i>=12 || (i<=6 && i>=3) || i==9){
	                      //wczytywanie nowego adresu do wektora przerwan
	                      address+=2;
	                      memory=(int*)address;
	                      data=*memory;
	                      data=(data-0x4000) + 0x248;
	                      FCTL3=FWKEY;
	                      FCTL1=FWKEY+WRT;
	                      *memory2=data;
	                      FCTL1=FWKEY;
	                      FCTL3=FWKEY+LOCK;

	                      address+=2;
	                  } else {
	                      FCTL3=FWKEY;
	                      FCTL1=FWKEY+WRT;
	                      switch(i){
	                      case 11:
	                          *memory2=TAIFG_ADDR;
	                          break;
	                      case 10:
	                          *memory2=TIMERA_ADDR;
	                          break;
	                      case 8:
	                          *memory2=TX_ADDR;
	                          break;
	                      case 7:
	                          *memory2=RX_ADDR;
	                          break;
	                      case 2:
	                          *memory2=NMI_ADDR;
	                          break;
	                      case 1:
	                          *memory2=RESET_ADDR;
	                          break;
	                      }
	                      FCTL1=FWKEY;
	                      FCTL3=FWKEY+LOCK;
	                      address+=4;
	                  }
	              }

	              __bis_SR_register(GIE);

	              //wyswietl wiadomosc o poprawnym wykonaniu i wlaczeniu programu z RAMu
	              IE1 |= UTXIE0;
	              i=0;
	              while(i<9){
	                  if(b  && (P3IN & BIT0)) {
	                	  b=0;
	                      U0TXBUF=text4[i];
	                      IE1 |= UTXIE0;
	                      i++;
	                  }
	              }
	              IE1 &= ~UTXIE0;
	              P2OUT=0x00;

	              TACTL &= ~(TAIE+MC_0);
	              ME1 &= ~(UTXE0 + URXE0);
	              IE1 &= ~(UTXIE0+URXIE0);

	              //flag2=0x00;
	              //wlacz program umieszczony w pamieci RAM
	              asm(" MOV.W #0248h, PC");
	          }
	      }
}

  #pragma vector = USART0TX_VECTOR
  __interrupt void TransmitInterrupt(void)
  {
      //IE1 &= ~UTXIE0;
      //IFG1 |= UTXIFG0;
      b=1;
      __bic_SR_register_on_exit(LPM0_bits);
  }

  #pragma vector = USART0RX_VECTOR
  __interrupt void ReceiveInterrupt(void)
  {
	    //pobierz nowy bajt
	    memory=(int*)address;
	    buffer[flag1]=U0RXBUF;

	    //wczytaj bajt do RAM
	    *memory=(buffer[0] << 8) + buffer[1];

	    //przejdz do kolejnego adresu i sutaw flage
	    if(flag1==1) address+=2;
	    flag1=(flag1+1) & 0x01;

	    //odblokuj TIMER
	    TACCTL0 |= CCIE;
	    TAR=0;

	    __bic_SR_register_on_exit(LPM0_bits);
  }

  //--------------------------------------------------------------przerwanie TIMERA---------------------------------------------------------
  //TIMER zluzy do sprawdzania czy dane sa wczytywane z pliku czy z klawiatury
  //jesli po wczytaniu bajtu kolejny bajt nie nadejdzie w czasie 10ms ozancza to ze dane sa wczytywane przez uzytkownika
  //taka sytuacja moze tez oznaczac ze nastapil koniec wczytywania pliku
  #pragma vector=TIMERA0_VECTOR
  __interrupt void TIME(void){
      //wyczysc flage wczytanych bajtow
      flag1=0;

      //przejdz do adresu poczatkowego
      address2=address;
      address=ADDR_BEGIN;

      //zablokuj przerwania TIMERA i wyczysc flagi
      //IE1 &= ~UTXIE0;
      TACCTL0 &= ~CCIFG;
      TACCTL0 &= ~CCIE;

      __bic_SR_register_on_exit(LPM0_bits);
  }

  //--------------------------------------------------------------przerwanie TAIFG---------------------------------------------------------
  //czysczenie flagi TAIFG
  #pragma vector=TIMERA1_VECTOR
  __interrupt void REFRESH(void){
      TACTL&=~TAIFG;
      __bic_SR_register_on_exit(LPM0_bits);
  }

  //-------------------------------------------------------------getData(...)--------------------------------------------------------------
  //pobranie danych w zaleznosci od przesuniecia
  int getData(int address,int i){
      int* raw;
      raw=(int*)address;
      int data=*raw;
      if(i==0) return data;
      else {
          data = (data << 8);
          raw  = (int*)(address+2);
          data += ((*raw >> 8) & 0x00FF);
          return data;
      }
  }

  //------------------------------------------------------------getByte(...)---------------------------------------------------------------
  //przetlumaczenie pobranych danych z ASCII na HEX
  char getByte(int word){
      char buf[2];
      buf[0]=(data>>8) & 0x00FF;
      buf[1]=(data & 0x00FF);

      if(buf[0]<0x41) buf[0]-=0x30;
      else buf[0]-=0x37;

      if(buf[1]<0x41) buf[1]-=0x30;
      else buf[1]-=0x37;

      return ((buf[0] << 4)+buf[1]);
  }
