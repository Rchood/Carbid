// Logging detail - 1-4
int logLevel = 1;

// Seriele communicatie
#define BIT_AMOUNT 10          // aantal bits
int startbitLength = 1500;    // milliseconden
int bitLength = 75;           // milliseconden
int pinVerzenden = 8;
bool verzendModus = false;
int bitvalue[BIT_AMOUNT];
int bit;

// OLED 132x32
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET 11 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Weegschaal
const byte LOADCELL_DOUT_PIN = 2;
const byte LOADCELL_SCK_PIN = 3;
const byte weegPin = 4;
const byte resetPin = 5;
#include <HX711.h>
HX711 scale;
int gewicht = 0;
long base = 420000; //waarde bij 0 gram (Tare)
int gewichtMeting = 0;

// Geluidsmeting (moet nog)

void setup() {
  Serial.begin(9600);

  // Seriele communicatie
  pinMode (8, OUTPUT);  // zendpin
  bitvalue[0] = 512;
  bitvalue[1] = 256;
  bitvalue[2] = 128;
  bitvalue[3] = 64;
  bitvalue[4] = 32;
  bitvalue[5] = 16;
  bitvalue[6] = 8;
  bitvalue[7] = 4;
  bitvalue[8] = 2;
  bitvalue[9] = 1;

  // Weegcel
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  pinMode (weegPin, INPUT);
  pinMode (resetPin, INPUT);

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while(1); // Don't proceed, loop forever
  }
}

void loop() {
  gewichtMeting = 0;
  msgDebug(4, String("Started loop"));
  // Zet verzenden uit totdat de weeging gedaan is
  digitalWrite (pinVerzenden, LOW);

  // Carbid weging
  while (gewichtMeting == 0) {
	msgDebug(2, String("gewichtsmeeting gestart"));
    gewichtMeting = weeging();
	//msgDebug(3, String("Gewogen gewicht = " + gewichtMeting + "gr."));
  }

  int SendingNumber = gewichtMeting;
  int BitState[BIT_AMOUNT];  // Array van de bitstatus [aantal bits]

  for (bit=0; bit < BIT_AMOUNT; bit = bit+1) {
    if (bitvalue[bit] <= SendingNumber) {
      BitState[bit] = 1;
      SendingNumber -= bitvalue[bit];
    } else {
      BitState[bit] = 0;
    }
  }

  // Verstuur gewogen gewicht
  digitalWrite (pinVerzenden, HIGH);
  delay (startbitLength);
  digitalWrite (pinVerzenden, LOW);   // wait
  delay (bitLength);

  for (bit=0; bit < BIT_AMOUNT; bit = bit+1) {
    if (BitState[bit] == 1) {       // als Bit AAN
      digitalWrite (pinVerzenden, HIGH);
              Serial.print ("bit: ");
        Serial.print (bit);
        Serial.print (" - Bit ON - ");
        Serial.println (millis()); 
    }else {                            // als Bit UIT
      digitalWrite (pinVerzenden, LOW);
              Serial.print ("bit: ");
        Serial.print (bit);
        Serial.print (" - Bit OFF - ");
        Serial.println (millis()); 
    }
    delay (bitLength);
  }
    gewichtMeting = 0;
    display.clearDisplay();
    display.setTextSize(2);               // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,10);              // Breedte, hoogte
    display.println("verzonden");
    display.display();                     // Show initial text
    delay(1000);
  
  
}

int weeging() {
  int gewichtMeting = 0;
  int gram = 0;
  long reading = scale.read();
  msgDebug(2, String("Weging gestart"));
    // stel nulpunt in
    if (digitalRead (resetPin) == HIGH) {base = reading;}
  
    gram = map(reading, base, (2051157 + base), 0, 1000);
    if (digitalRead (weegPin) == HIGH) {gewichtMeting = gram;}
  
//    msgDebug(3, String("gewogen gewicht: " + gram + " - GewichtsMeting: " + gewichtMeting));
  
    // OLED weergave
    display.clearDisplay();
    display.setTextSize(2);               // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20,0);              // Breedte, hoogte
    display.println(gram);
    display.setCursor(80,0);
    display.println("gram");
    display.setCursor(20,16);              // Breedte, hoogte
    display.println(gewichtMeting);
    display.setCursor(80,16);
    display.println("gram");
    display.display();                     // Show initial text
    delay(100);
 
  
  return gewichtMeting;
}

void msgDebug(int verbose, String message) {
  if (verbose <= logLevel) {Serial.println(message);}
}
