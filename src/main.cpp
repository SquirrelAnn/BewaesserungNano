#include <Arduino.h>

#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_BW<GxEPD2_290_GDEY029T94, MAX_HEIGHT(GxEPD2_290_GDEY029T94)> display(GxEPD2_290_GDEY029T94(/*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*BUSY=*/ 7)); // GDEY029T94  128x296, SSD1680, (FPC-A005 20.06.15)

// E paper settings for Arduino Nano or Uno
// BUSY -> D7
// RST -> D9
// DC -> D8
// CS -> D10
// CLK -> D13
// DIN -> D11
// GND -> GND
// 3.3V -> 3.3V

// last time the sensors were measured, in milliseconds
unsigned long previousMillis = 0;
// for sensor measuring and possible pump interval
const long measuringInterval = 18000000; // 5 hours

// water pumps to relay
int IN1 = 2;
int IN2 = 3;
int IN3 = 4;
int IN4 = 5;

// soil moisture sensors
int Pin1 = A0;
int Pin2 = A1;
int Pin3 = A2;
int Pin4 = A3;

// measured values of soil humidity
float soilHumid1 = 0.0;
float soilHumid2 = 0.0;
float soilHumid3 = 0.0;
float soilHumid4 = 0.0;

void setup() {
  Serial.begin(9600);
  //put your setup code here, to run once:
  display.init();

  analogReference(EXTERNAL); // set the analog reference to 3.3V

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(Pin1, INPUT);
  pinMode(Pin2, INPUT);
  pinMode(Pin3, INPUT);
  pinMode(Pin4, INPUT);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);

  delay(500);
}

bool water(int pumpNumber){
  digitalWrite(pumpNumber, LOW); // turn on pump
  delay(30000); // 30 seconds pump time
  digitalWrite(pumpNumber, HIGH); // turn off pump
}

void writeToEPaper(const char* msg){ 
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  if (display.epd2.WIDTH < 104){
     display.setFont(0);
  }
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(msg, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(msg);
  }
  while (display.nextPage());

  display.powerOff();
}

String pumpIfDry(float sensorValue, int sensorNumber, int pumpNumber){
  Serial.print("Soil Moisture Sensor ");
  Serial.print(sensorNumber);
  Serial.print(" Voltage: ");
  Serial.print(sensorValue);
  Serial.println(" V"); // print the voltage

  // 1.3V and lower --> super wet soil --> no water needed
  // 2.3V and higher --> super dry soil --> water needed
  // 2.75 or higher --> probably sensor is not in soil but in the air, don't water
  if (sensorValue >= 2.3 && sensorValue <= 2.7){
    Serial.println("Watering...");
    water(pumpNumber);

    String pumpNo = "Pump";
    String waterMsg = ": Watering finished.";
    String returnMsg = pumpNo +  (pumpNumber-1) + ":" + sensorValue + waterMsg;
    return returnMsg;
  }
  else{
    String pumpNo = "Pump";
    String waterMsg = " No watering.";
    String returnMsg = pumpNo + (pumpNumber-1) + ":" + sensorValue + waterMsg;
    return returnMsg;
  }  
}



float calcSoilHumid(int sensorPin){
  return (float(analogRead(sensorPin))/1023.0)*3.3;
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= measuringInterval 
    || previousMillis == 0) // if it's time to measure or first time
  {
    previousMillis = currentMillis;

    // read sensor values, print them and water if necessary
    soilHumid1 = calcSoilHumid(Pin1);
    String EpaperMsg1 = pumpIfDry(soilHumid1, 1, IN1);
    Serial.println(pumpIfDry(soilHumid1, 1, IN1));

    soilHumid2 = calcSoilHumid(Pin2);
    String EpaperMsg2 = pumpIfDry(soilHumid2, 2, IN2);
    Serial.println(pumpIfDry(soilHumid2, 2, IN2));

    soilHumid3 = calcSoilHumid(Pin3);
    String EpaperMsg3 = pumpIfDry(soilHumid3, 3, IN3);
    Serial.println(pumpIfDry(soilHumid3, 3, IN3));

    soilHumid4 = calcSoilHumid(Pin4);
    String EpaperMsg4 = pumpIfDry(soilHumid4, 4, IN4);
    Serial.println(pumpIfDry(soilHumid4, 4, IN4));
   
    String msgComplete = "\n" + EpaperMsg1 
    + "\n" + EpaperMsg2
    + "\n" + EpaperMsg3
    + "\n" + EpaperMsg4;

    // const char* str = EpaperMsg1.c_str();
    const char* str = msgComplete.c_str();
  
    writeToEPaper(str);
  }  
}
