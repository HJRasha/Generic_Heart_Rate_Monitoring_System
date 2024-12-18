#include <Wire.h>
#include <TinyGPS++.h>
#include <LoRa.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// To send Emails using Gmail on port 465 (SSL), you need to create an app password: https://support.google.com/accounts/answer/185833
#define emailSenderAccount    "example_sender_account@gmail.com"
#define emailSenderPassword   "email_sender_password"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[ALERT] ESP32 Temperature"

// Default Recipient Email Address
String inputMessage = "your_email_recipient@gmail.com";
String enableEmailChecked = "checked";
String inputMessage2 = "true";
// Default Threshold Temperature Value
String inputMessage3 = "25.0";
String lastTemperature;

TinyGPSPlus gps;

#define SS_PIN    18
#define RST_PIN   14
#define DI0_PIN   26

#define BAND    915E6  // You may need to adjust this based on your LoRa module frequency

// Replace with your LoRa module's address
#define DEVADDR 0x26011B48

// Replace with your LoRa module's network session key
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, /* ... */ };

// Replace with your LoRa module's application session key
static const PROGMEM u1_t APPSKEY[16] = { 0x00, /* ... */ };

// Replace with your LoRa module's device address (in big endian format)
static const u4_t DEVADDR32 = 0x26011B48;

void setup() {
  Serial.begin(115200);

  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (1);
  }

  LoRa.setRxSingle(); // Set LoRa to receive single packets

  Serial2.begin(9600); // Initialize the Serial2 for GPS communication
}

void loop() {
  // Receive data via LoRa
  if (LoRa.parsePacket()) {
    String receivedData = "";

    while (LoRa.available()) {
      char c = LoRa.read();
      receivedData += c;
    }

    // Parse and display received data
    parseAndDisplayData(receivedData);
  }

  // Update GPS data
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      // You can process and display GPS data here if needed
    }
  }

  // You can add other tasks or functions here
}

void parseAndDisplayData(String data) {
  Serial.println("Received Data: " + data);

  // Parse the data (assuming the format "heartRate,temperature,latitude,longitude,altitude")
  int heartRate = data.substring(0, data.indexOf(',')).toInt();
  data = data.substring(data.indexOf(',') + 1);
  
  float temperature = data.substring(0, data.indexOf(',')).toFloat();
  data = data.substring(data.indexOf(',') + 1);

  float latitude = data.substring(0, data.indexOf(',')).toFloat();
  data = data.substring(data.indexOf(',') + 1);

  float longitude = data.substring(0, data.indexOf(',')).toFloat();
  data = data.substring(data.indexOf(',') + 1);

  float altitude = data.toFloat();

  // Display parsed data
  Serial.println("Heart Rate: " + String(heartRate));
  Serial.println("Temperature: " + String(temperature));
  Serial.println("Latitude: " + String(latitude, 6));
  Serial.println("Longitude: " + String(longitude, 6));
  Serial.println("Altitude: " + String(altitude));
}

