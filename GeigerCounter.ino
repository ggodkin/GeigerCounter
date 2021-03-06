
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET  -1 //   4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  int countTime = 20; //Seconds
  const byte interruptPin = 2;
  volatile long countPulse = 0;
  volatile long oldTime = 0;
  volatile long newTime = 0;
  int arrayPosition = 0;
  const int arraySize = 9;
  int impCount[arraySize];
  int nonZeroCount = 0;
  long totCountPulse = 0;
  int i;

// the setup function runs once when you press reset or power the board
void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), counterGeiger, FALLING);

  pinMode(9, OUTPUT);
  analogReference(DEFAULT); //power supply
  
  TCCR1A = _BV(COM1A0);  //CTC
  TCCR1B = _BV(CS11) | _BV(WGM12); //clkT2S/8 (from prescaler) OC1A toggle
  OCR1A = 28;

  for (i = 0; i < arraySize; i++) {
    impCount[i] = 0;
  }
  
  Serial.begin(115200);
  Serial.println("Setup completed");

  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Setup\ncompleted"));

  display.display();
}

void loop() {
  newTime = millis();

  if ((newTime - oldTime) >= countTime *1000) {
    oldTime = newTime;
    
    impCount[arrayPosition] = countPulse;
    arrayPosition ++;
    if (arrayPosition >= arraySize) {
      arrayPosition = 0;
    }
    nonZeroCount = 0;
    totCountPulse = 0;
    for (i = 0; i < arraySize; i++) {
      if (impCount[i] > 0 ) {
        nonZeroCount ++;
        totCountPulse += impCount[i];
      }
    }
    int avgPPM = totCountPulse * 60 /(countTime * nonZeroCount);
    int avgRad = avgPPM * 60 / 67;
    
    Serial.println("Time, s: " + String(oldTime/1000)); //prints time since program started
    Serial.println("arraySize " + String(arraySize));
    Serial.println("Count, pulses/" + String(countTime) + " sec: " + String(countPulse));
    Serial.println("uR/hr (Cs 137): " + String(countPulse * 3600 / 65 / countTime));
    Serial.println("totCountPulse " + String(totCountPulse));
    Serial.println("countTime " + String(countTime));
    Serial.println("nonZeroCount " + String(nonZeroCount));
    Serial.println("avgPPM " + String(avgPPM));
    Serial.println("avgRad " + String(avgRad));
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    displayPrintln("  Geiger Counter",1);
    displayPrintln("Current/Average uR/hr",1);
    //displayPrintln(String(countPulse*60/countTime) + ' ' + String(countPulse * 3600 / 67 / countTime),2);
    //displayPrintln("Average PPM uR/hr",1);
    displayPrintln(String(countPulse * 3600 / 67 / countTime),2); // Current radiation
    displayPrintln(String(avgRad),3);   //Average radiation
    display.display();
    countPulse = 0;
  } else if (newTime < oldTime) {
    oldTime = newTime;
    countPulse = 0;
    Serial.println ("Resetting - runtime is over 50 days");
  }
}

void counterGeiger(){
  countPulse ++;
}

void displayPrintln(String str2Display, int txtSize) {
  displayPrint(str2Display,txtSize);
  display.println();
}
void displayPrint(String str2Display, int txtSize) {
  display.setTextSize(txtSize);
  display.print(str2Display);
}
