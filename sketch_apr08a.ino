#include <Wire.h>
#include <LiquidCrystal.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))
/*Kalibracja czujników*/
#define T_OFFSET 1
#define P_OFFSET 1850
#define SENSOR_ADDRESS  0x76 //Trzeba było zdefiniować adres czujnika 

Adafruit_BMP280 czujnik; //inicjalizacja czujnika tempratury i ciśnienia


LiquidCrystal lcd(7, 6, 5, 4, 3, 2); //(RS, E, D4, D5, D6, D7) 
ThreeWire myWire(A1, A0, A2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
/*Deboucing*/
long lastDebounceT = 0;
long debounceDelay = 300;
bool DisplayChangeMem = 0;
/*Zmienne do działania czujnika na timerze */
long lastMessureTP = 0;
long lastMessureTime = 0;
long messureDelay = 1000;
/*Pin podłączony do przełącznika zmieniającego informacje na ekranie*/
const short ButtonPin = 8;
short displayNo = 1; //Numer początkowy ekranu 
bool buttonState = LOW;

void setup() {

  pinMode(ButtonPin, INPUT);
  
  RtcDateTime Czas = RtcDateTime(__DATE__, __TIME__);
  
  Serial.begin(9600);
  
  Rtc.Begin();   
  //Rtc.SetDateTime(Czas);

  lcd.begin(16, 2);
  if (!czujnik.begin(SENSOR_ADDRESS))
  {
    Serial.println("Nie znaleziono czujnika!");
  }
  Serial.print("Data:");
  Serial.println(__DATE__);
  Serial.print("Czas:");
  Serial.println(__TIME__);
}
void loop()
{
buttonState = digitalRead(ButtonPin);
/*Funckja filtrująca drgania styków oraz przełączająca ekran, funkcja pozwala zmienić ekran raz na kliknięcie niezależnie od czasu trzymania przycisku*/
  if ( (millis() - lastDebounceT) > debounceDelay) {
    if ( (buttonState == HIGH) && (displayNo == 1)&&(DisplayChangeMem == 0) ) {
      displayNo = 2;
      DisplayChangeMem = 1;
      lastDebounceT = millis(); //set the current time
    }
       else if ( (buttonState == HIGH) && (displayNo == 2) && (DisplayChangeMem == 0)){
      displayNo = 1;
      DisplayChangeMem = 1; 
      lastDebounceT = millis();
    }
  else if ((buttonState == LOW)&&(DisplayChangeMem == 1)) DisplayChangeMem = 0; 
  }
 /*Przełączanie ekranu w zależności od zmiennej displayNo.*/
switch(displayNo)
{
  case 1:
    PrintTemperaturePressure();
    break;
  case 2: 
    printDateTime(Rtc.GetDateTime());
    break;
} 
}
/*Odczyt temperatury z czujnika*/
void PrintTemperaturePressure() 
{
  char txt[14];
  float temperature;
  float pressure;
  
  if ((millis() - lastMessureTP) > messureDelay){
    lcd.clear();
    /*Dodanie wartości OFFSET*/
    temperature = czujnik.readTemperature() - T_OFFSET;
    pressure = czujnik.readPressure()+ P_OFFSET;
     
  
    sprintf(txt, "%d.%02u%cC  ", (int)(temperature), (int)(temperature * 100) % 100, 223);
    lcd.setCursor(4, 0);
    lcd.print(txt);
  
    sprintf(txt, "%u.%02uhPa ", (int)(pressure / 100), (int)((uint32_t)pressure % 100));
    lcd.setCursor(3, 1);
    lcd.print(txt);
    
    lastMessureTP = millis();
  }
}
/*Odczyt z zegara czasu rzeczywistego*/
void printDateTime(const RtcDateTime& dt) {

 if ((millis() - lastMessureTime) > messureDelay){
  lcd.clear();
  lcd.setCursor(3, 0);
  char datestring[20];
  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u"),
             dt.Day(), dt.Month(), dt.Year());
  lcd.print(datestring);
  lcd.setCursor(4, 1);
  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u:%02u:%02u"),
             dt.Hour(), dt.Minute(), dt.Second());
  lcd.print(datestring);
  
  lastMessureTime = millis();
 }
}
