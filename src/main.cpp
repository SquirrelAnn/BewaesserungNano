#include <Arduino.h>

// for sensor measuring and possible pump interval
unsigned long previousMillis = 0;
const long measuringInterval = 18000000; // 5 hours

// for water pumps to relay
int IN1 = 2;
int IN2 = 3;
int IN3 = 4;
int IN4 = 5;

// for water level sensors
int Pin1 = A0;
int Pin2 = A1;
int Pin3 = A2;
int Pin4 = A3;

// for soil moisture sensors
float soilHumid1 = 0.0;
float soilHumid2 = 0.0;
float soilHumid3 = 0.0;
float soilHumid4 = 0.0;

void setup() {
  Serial.begin(9600);

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

String pumpIfDry(float sensorValue, int sensorNumber, int pumpNumber){
  Serial.print("Soil Moisture Sensor ");
  Serial.print(sensorNumber);
  Serial.print(" Voltage: ");
  Serial.print(sensorValue);
  Serial.println(" V"); // print the voltage

  // 1.3V and lower --> super wet soil --> no water needed
  // 2.3V and higher --> super dry soil --> water needed
  // 2.75 or higher --> probably sensor is not in soil but in the air, don't water
  if (sensorValue >= 2.3 && sensorValue <= 2.75){
    Serial.println("Watering...");
    water(pumpNumber);
    return "Watering for 30 seconds finished.";
  }
  else{
    return "No watering needed.";
  }  
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= measuringInterval 
    || previousMillis == 0) // if it's time to measure or first time
  {
    previousMillis = currentMillis;

    // read sensor values, print them and water if necessary
    soilHumid1 = (float(analogRead(Pin1))/1023.0)*3.3;
    Serial.println(pumpIfDry(soilHumid1, 1, IN1));

    soilHumid2 = (float(analogRead(Pin2))/1023.0)*3.3;
    Serial.println(pumpIfDry(soilHumid2, 2, IN2));

    soilHumid3 = (float(analogRead(Pin3))/1023.0)*3.3;
    Serial.println(pumpIfDry(soilHumid3, 3, IN3));

    soilHumid4 = (float(analogRead(Pin4))/1023.0)*3.3;
    Serial.println(pumpIfDry(soilHumid4, 4, IN4));
  }  
}
