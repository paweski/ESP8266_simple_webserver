#include <dht11.h>
#include <SoftwareSerial.h>

const byte rxPin = 2;
const byte txPin = 3;
const int dht11Pin = 9;

dht11 czujnikTemperatury;
int statusCzujnika = 5;

char klient[1];
char bufor[150];
String strona_www ="";
unsigned long czas_do_pomiaru = 15; // <- (tu zmień)odstęp między pomiarami w [s]
int temperatura, wilgotnosc;
unsigned int t0, t1;

// set up a new serial object
SoftwareSerial Serial1(rxPin, txPin);


void setup() 
{

// pinmode setup
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  //baud rate setup
  Serial.begin(9600);
  while(!Serial);
  Serial1.begin(57600);

while(!wyslij("AT","OK",100))
  {
    Serial.println(" Błąd komunikacji z modułem wifi! ");
    delay(1000);
  }
  Serial.println("Rozpoczęto komunikację z modułem wifi");


if ( wyslij("AT+CWMODE=1","OK",100) )
  Serial.println("CWMODE=1 - OK");
  

/* Często CIPMODE jest już zapisana w module i nie potwierdzany
* przy ponownym ustawieniu, więc tu może nie odpowiedzieć OK
*/

if ( wyslij("AT+CIPMODE=0","OK",100) )
  Serial.println("CIPMODE=0 - OK");

if ( wyslij("AT+CIPMUX=1","OK",100) )
  Serial.println("CIPMUX=1 - OK");

 if ( wyslij("AT+CWJAP=\"UPC241087269\",\"A4HXGL62\"","OK",5000 ) )
  Serial.println("Połączono z siecią UPC241078269 :-)");

  if ( wyslij("AT+CIPSERVER=1,80","OK",1000) )
  Serial.println("Server wystartował!");

  Serial1.println("AT+CIFSR");                  
  Serial.println(Serial1.readString());

  t0 = millis();
           
}



void loop() 
{
  // put your main code here, to run repeatedly:

  t1 = millis();

  if ( ( t1 - t0 ) > ( czas_do_pomiaru * 1000 ) )
  {
  
  t0 = millis();
  statusCzujnika = czujnikTemperatury.read( dht11Pin );

      if  ( statusCzujnika == DHTLIB_OK ) 
        {
            temperatura =  czujnikTemperatury.temperature;
            wilgotnosc = czujnikTemperatury.humidity  ;
        }      
  }

  while ( Serial1.available() > 0 )
  {
    if( Serial1.find( "+IPD," ) )
    {
      Serial1.readBytesUntil( ',', klient,1 );
      Serial.println();
      Serial.print( "Zapytanie ID: " );
      Serial.println( klient[0] );
  
      strona_www = "<html lang=\"pl\"/><head><meta charset=\"utf-8\" /><title>Stryjeńskich 17 :)</title>" ;
      strona_www += "</head><body> <h1>" ;

      char str[35];
      sprintf( str, "Aktualna temperatura: +%dC, wilgotność %d%s",temperatura, wilgotnosc, "%" );
      
      strona_www += str;
     
      strona_www += "</h1> <img src=\"http://motormania.com.pl/wp-content/uploads/2017/11/Honda-Africa" ;
      strona_www += "-Twin-CRF-1000-L-Africa-Twin-2018-12.jpg\" width=\"800\" height=\"550\"></body></html>" ; 


         sprintf( bufor , "AT+CIPSEND=%c,%d", klient[0], strona_www.length() );

         if ( wyslij( bufor, ">" , 100 ) )
           Serial.println("Żądanie wysłania ...");

        if ( wyslij(strona_www, "OK", 100 ) )
           Serial.println("Dane wysłane ");

        sprintf( bufor , "AT+CIPCLOSE=%c", klient[0] );
        
        if  ( wyslij( bufor , "OK", 100 ) )
        {
          Serial.print("Zamknąłem połączenie z klientem:");
          Serial.println(klient[0]);
        }
        
    }
    
  }
}



boolean wyslij( String komenda_AT, char *Odpowiedz_AT, int czas_czekania )
{
  Serial1.println( komenda_AT );
  delay( czas_czekania );
  while ( Serial1.available() > 0)
  {
    if ( Serial1.find( Odpowiedz_AT ) )
      return 1;
  }
  return 0;
}

