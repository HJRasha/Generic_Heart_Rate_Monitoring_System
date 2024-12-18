#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <TinyGPS++.h>
#include <LoRa.h>

MAX30105 particleSensor;
TinyGPSPlus gps;

//define the pins used by the LoRa transceiver module
#define ss 5
#define rst 14
#define dio0 2
#define BAND 433E6  // You may need to adjust this based on your LoRa module frequency

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

void setup() {
  Serial.begin(115200);

  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (1);
  }

  LoRa.setTxPower(20); // Set transmit power for LoRa module

  // Initialize the MAX3010x sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }

  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  Serial2.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); // Initialize the Serial2 for GPS communication
}

void loop() {
  // Read heart rate and temperature data
  int heartRate = 0;
  float temperature = 0.0;

  if (particleSensor.available()) {
    heartRate = particleSensor.getHeartRate();
    temperature = particleSensor.readTemperature();
  }

  // Read GPS data
  float latitude = 0.0;
  float longitude = 0.0;
  float altitude = 0.0;

  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      altitude = gps.altitude.meters();
    }
  }

  // Send data via LoRa
  sendLoRaData(heartRate, temperature, latitude, longitude, altitude);

  delay(1000); // Adjust the delay based on your requirements
}

void sendLoRaData(int heartRate, float temperature, float latitude, float longitude, float altitude) {
  // Prepare data to send
  String data = String(heartRate) + "," + String(temperature) + "," + String(latitude, 3) + "," + String(longitude, 3) + "," + String(altitude, 2);

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&data[0], data.length());
  LoRa.endPacket();
  Serial.println("Data sent via LoRa: " + data);
}

