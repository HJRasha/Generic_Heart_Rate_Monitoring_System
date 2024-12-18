#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <TinyGPS++.h>
#include <SPI.h>
#include <LoRa.h>

#define BAND 433E6  // You may need to adjust this based on your LoRa module frequency
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

int counter = 0;

MAX30105 particleSensor;
TinyGPSPlus gps;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float heartRate;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(BAND)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(0); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY(); //Enable the temp ready interrupt. This is required.

  Serial2.begin(9600);
}

void loop() {
  long irValue = particleSensor.getIR();
  float temperature = particleSensor.readTemperature();

  Serial.print("temperatureC=");
  Serial.print(temperature, 4);

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();
    heartRate = 60 / (delta / 1000.0);
  }

  Serial.print(", heartRate=");
  Serial.print(heartRate);

  if (irValue < 50000)
    Serial.print(" No finger?");
  Serial.println();

  // Read GPS data
  float latitude = 0.0;
  float longitude = 0.0;
  float altitude = 0.0;

  if (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        Serial.print(F("- latitude: "));
        Serial.println(latitude, 3);
        longitude = gps.location.lng();
        Serial.print(F("- longitude: "));
        Serial.println(longitude, 3);

        if (gps.altitude.isValid())
          altitude = gps.altitude.meters();
        else
          Serial.println(F("INVALID"));
      }
    }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

  sendLoRaData(heartRate, temperature, latitude, longitude, altitude);
}

void sendLoRaData(int heartRate, float temperature, float latitude, float longitude, float altitude) {
  // Prepare data to send
  String data = String(heartRate) + "," + String(temperature) + "," + String(latitude, 3) + "," + String(longitude, 3) + "," + String(altitude, 2);
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
  delay(100);
}

