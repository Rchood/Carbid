// Delays
long delayArmed = 1000;
long delayCleaning = 5000;
long delayOntsteking = 3000;

// Seriele communicatie
#define BIT_AMOUNT 10  // aantal bits
int bitValue[BIT_AMOUNT];  // aantal bits wat wij gaan ontvangen
int pinOntvangen = 6;  //  pin waarop de communicatie ontvangen wordt

// Gewicht ontvangen
int resultaat = 2;
int gewicht = 0;

// Bus klaar knop
int pinStartSein = 2;

// Zwaailamp relay
int pinZwaailamp = 4;

// Ontsteking relay
int pinOntsteking = 3;

// Variabele voor het vorige gewicht
int gewichtOld = 0;
int pinGewichtOld = 7;

// Variabelen voor de tijden
long wachtTijd = 0;
static unsigned long startTime = 0;
static unsigned long zwaailampTime = 0;
static unsigned long ingitionTime = 0;
int wachtTijdOffset = 0;
unsigned long countDown;

// OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  Serial.begin(9600);

  // Pinmodus instellen
  pinMode (pinOntsteking, OUTPUT);
  pinMode (pinZwaailamp, OUTPUT);
  pinMode (pinStartSein, INPUT);
  pinMode (pinOntvangen, INPUT);
  pinMode (pinGewichtOld, INPUT);

  // Assign bitValue
  bitValue[0] = 512;
  bitValue[1] = 256;
  bitValue[2] = 128;
  bitValue[3] = 64;
  bitValue[4] = 32;
  bitValue[5] = 16;
  bitValue[6] = 8;
  bitValue[7] = 4;
  bitValue[8] = 2;
  bitValue[9] = 1;
  
  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextSize(2);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,10);  // Breedte, hoogte
  display.println("Carbid programmma");
  display.display();  // Show initial text
  delay(1000);
}

void loop() {
  digitalWrite(pinZwaailamp, HIGH);
  digitalWrite(pinOntsteking, HIGH);
  gewicht = 0;
  Serial.println(F("Programma is klaar voor gebruik!"));

  display.clearDisplay();
  display.setTextSize(2);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,10);  // Breedte, hoogte
  display.println("Klaar voor gebruik");
  display.display();  // Show initial text
  delay(1000);

  
  // Ontvang het gewicht vanuit de nano
  if (gewicht == 0) {
    gewicht = gewichtOntvangen();
    Serial.print("ontvangen gewicht: ");
    Serial.println(gewicht);
  }
  
  // Ga in een loop totdat we een resultaat hebben
  while (resultaat == 2){
    // Als de bus geladen
    if (digitalRead(pinStartSein) == HIGH) {
      // Ga de benodigde tijden berekenen
	    wachtTijd = calcTime(gewicht);
      if (wachtTijd < 0) {
        resultaat = 1;
      } else {
      Serial.print("WachtTijd = ");
	    Serial.println(wachtTijd);
	    startTime = millis();
	    zwaailampTime = (startTime + ((wachtTijd - 5) *1000));
	    ingitionTime = (startTime + (wachtTijd * 1000));
      resultaat = armed(zwaailampTime, ingitionTime);
    }}
  }

  // Bepaal of we een succesvolle ontsteking gehad hebben of dat we het hebben afgebroken
  if (resultaat == 0) {
	  Serial.println(F("Sucessvol ontsteking"));
	  cleaning(gewicht);
  } else if (resultaat == 1) {
	  Serial.println(F("Ontstekning afgebroken"));
	  cleaning(gewicht);
  }
}

int armed(long zwaailampTime, long ingitionTime) {
  delay(delayArmed);

    display.clearDisplay();
    display.setTextSize(2);  // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,10);  // Breedte, hoogte
    display.print("Aftellen ");
    display.setCursor(0,30);  // Breedte, hoogte
    display.print("begonnen!");
    display.display();  // Show initial text
    //delay(1000);

                                                               
  Serial.println(F("Aftellen begonnen :-)"));
  while (1) {
    // Annuleer het aftellen en stop de ontsteking
    if (digitalRead(pinStartSein) == HIGH) {
      return 1;
    }
    
    countDown = ((ingitionTime - millis())/1000);  
    Serial.print ("countdown!  ");
    Serial.println (countDown);
    display.clearDisplay();
    display.setTextSize(2);  // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,10);  // Breedte, hoogte
    display.print("Countdown! ");
    display.setCursor(50,30);  // Breedte, hoogte
    display.print(countDown);
    display.display();  // Show initial text
    
    // Zwaailamp aanzetten
    if ((millis() >= zwaailampTime) && (zwaailampTime != 0)) {
      digitalWrite(pinZwaailamp, LOW);
    }

    // Ontsteking incl. geluidsmeting starten
    if ((millis() >= ingitionTime) && (ingitionTime != 0)) {
      digitalWrite(pinOntsteking, LOW);
      Serial.println(F("ontsteking actief, delay geactiveerd"));
      drawcircle();    // Draw circles (outlines)

      delay(delayOntsteking);
      digitalWrite(pinOntsteking, HIGH);
      Serial.println(F("ontsteking gedeactiveerd"));
      
      return 0;
    }
  }
}

void cleaning(int gewicht) {
  Serial.println(F("Cleaning process started"));
  digitalWrite(pinZwaailamp, HIGH);
  digitalWrite(pinOntsteking, HIGH);
  gewichtOld = gewicht;
  gewicht = 0;
  resultaat = 2;
  wachtTijd = 0;
  startTime = 0;
  zwaailampTime = 0;
  ingitionTime = 0;
  delay (delayCleaning);
  Serial.println(F("Cleaning process done"));
}

int calcTime(int gewicht) {
  int iniWachtTijd = 0;
  int wachtTijdOffset = 0;
  int wachtTijd = 0;
  iniWachtTijd = map(gewicht, 200, 300, 240, 220);
  wachtTijdOffset = map(analogRead(A0), 0, 1023, -120, 120);
  Serial.print("Wachttijd offset = ");
  Serial.println(wachtTijdOffset);
  wachtTijd = (iniWachtTijd + wachtTijdOffset);
  return wachtTijd;
}

int gewichtOntvangen() {
  int bitState[BIT_AMOUNT];  // Aray waar de Bitstatussen in komen [aantal bits]
  int received = 0;
  int rbit = 0;
  int readModus = 0;  // 0=uit; 1=paraat; 2=lezen;
  int startBitLength = 1500;  // aantal milliseconden van de startbit (moet langer zijn dan de totale te verzenden waarde)
  unsigned long t1 = 0;  // eerste timer voor de startbit
  unsigned long t2 = 0;  // tweede timer voor de startbit
  int bitLength = 75;  // aantal milliseconden van de te ontvangen bits

  // Zolang we nog geen waarde hebben ontvangen/berekend blijf in deze loop
  while (received == 0) {
    
    // Als we besluiten het oude gewicht te bereiken
    if (digitalRead(pinGewichtOld) == HIGH) {
      received = gewichtOld;
    }

    // timing 1 voor bepaling startbit
    if ((digitalRead(pinOntvangen) == HIGH) && (readModus == 0)) {
      readModus = 1;
      t1 = millis();
    }

    // timing 2 voor bepaling startbit
    if ((digitalRead (pinOntvangen) == LOW) && (readModus == 1)) {
      readModus = 2;
      t2 = millis();
    }
    
    // Controleer of we de startbit ontvangen hebben
    if (((t2 - t1 >= (startBitLength - 20)) || (t2 - t1 <= (startBitLength + 20))) && (readModus == 2)) {
      // Wacht 1.5x de bitlengte zodat we ongeveer midden in de bit zitten tijdens lezen
      delay (bitLength * 1.5);
  	  // Start met het inlezen van de bits en zet ze in de array
      for (rbit=0; rbit < BIT_AMOUNT; rbit++) {
        if (digitalRead (pinOntvangen) == HIGH) {
          bitState[rbit] = 1;
          Serial.print (" bit: ");
          Serial.print (rbit);
          Serial.println (" = Bit ON");
        } else {
          bitState[rbit] = 0;
          Serial.print (" bit ");
          Serial.print (rbit);
          Serial.println (" = Bit OFF");
        }
        delay (bitLength);
      }
      
      // berekenen de ontvangen waarde
      for (rbit=0; rbit < BIT_AMOUNT; rbit++) {
        received += (bitValue[rbit] * bitState[rbit]);
      }

      // reset gebruikte variabele
      t1 = 0;
      t2 = 0;
      readModus = 0;
    }

    wachtTijdOffset = map(analogRead(A0), 0, 1023, -120, 120);
    display.clearDisplay();
    display.setTextSize(2);  // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);  // Breedte, hoogte
    display.print("Offset: ");
    display.setCursor(90,00); // Breedte, hoogte
    display.println(wachtTijdOffset);
    display.setCursor(0,20);  // Breedte, hoogte
    display.print("Gew.  : ");
    display.setCursor(90,20); // Breedte, hoogte
    display.println(received);
    display.display();  // Show initial text
    //delay(1000);
  }
    return received;
}

void drawcircle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
    display.display();
    delay(20);
  }

  delay(2000);
}
