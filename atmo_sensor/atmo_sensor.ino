/*
 * Copyright (C) Samuel Jones - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Samuel Jones <spjones329@gmail.com>, February 2024
 */

#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include "Arduino_LED_Matrix.h"
#include "WiFiS3.h"
#include "arduino_secrets.h"

// define these in "arduino_secrets.h"
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;

ArduinoLEDMatrix matrix;
SensirionI2CScd4x scd4x;
IPAddress remoteHost(10, 0, 0, 70);
uint16_t portNumber = 8080;
WiFiClient client;

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

byte one[8][4] = {
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 1, 1, 1}};
byte two[8][4] = {
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 1}};
byte three[8][4] = {
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1}};
byte four[8][4] = {
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1}};
byte five[8][4] = {
    {0, 1, 1, 1},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1}};
byte six[8][4] = {
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1}};
byte seven[8][4] = {
    {0, 1, 1, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1}};
byte eight[8][4] = {
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1}};
byte nine[8][4] = {
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 0, 1}};
byte zero[8][4] = {
    {0, 1, 1, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1}};

byte frame[8][12] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

/**
 * @param num Number 0-9 to write
 * @param position Position 1, 2, or 3 (right to left) to write number in frame
 */
void writeNumberToFrame(byte num, byte position)
{
  byte columnOffset = (position - 1) * 4;
  byte(*numToCopy)[8][4];
  switch (num)
  {
  case 1:
    numToCopy = &one;
    break;
  case 2:
    numToCopy = &two;
    break;
  case 3:
    numToCopy = &three;
    break;
  case 4:
    numToCopy = &four;
    break;
  case 5:
    numToCopy = &five;
    break;
  case 6:
    numToCopy = &six;
    break;
  case 7:
    numToCopy = &seven;
    break;
  case 8:
    numToCopy = &eight;
    break;
  case 9:
    numToCopy = &nine;
    break;
  case 0:
    numToCopy = &zero;
    break;
  default:
    Serial.println("Warning: tried to encode number >9");
    return;
  }
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      frame[i][j + columnOffset] = (*numToCopy)[i][j];
    }
  }
}

void encodeFrame(uint16_t num)
{
  const byte ones = num % 10;
  num /= 10;
  const byte tens = num % 10;
  num /= 10;
  const byte hundreds = num % 10;
  num /= 10;
  const byte thousands = num;

  writeNumberToFrame(hundreds, 1);
  writeNumberToFrame(tens, 2);
  writeNumberToFrame(ones, 3);
  for (int i = 0; i < thousands; i++)
  {
    frame[i][0] = 1;
  }
}

void printUint16Hex(uint16_t value)
{
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2)
{
  Serial.print("Serial: 0x");
  printUint16Hex(serial0);
  printUint16Hex(serial1);
  printUint16Hex(serial2);
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(100);
  }
  Serial.println("\nHello from air quality sensor!");

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  printWifiStatus();

  if (!client.connect(remoteHost, portNumber))
  {
    Serial.println("Failed to connect to host");
  }

  matrix.begin();
  Wire1.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire1);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error)
  {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error)
  {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    printSerialNumber(serial0, serial1, serial2);
  }

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error)
  {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");
}

void sendSample(uint16_t co2, uint16_t temp, uint16_t humid)
{
  bool writable = false;
  if (!client.connected())
  {
    if (client.connect(remoteHost, portNumber))
    {
      Serial.print("Disconnected from host but successfully reconnected");
      writable = true;
    }
    else
    {
      Serial.println("Not connected to host and failed to reconnect");
    }
  }
  else
  {
    writable = true;
  }

  if (!writable)
    return;

  uint8_t bytes[9] = {co2, co2 >> 8, ' ', temp, temp >> 8, ' ', humid, humid >> 8, '\n'};
  client.write(bytes, 9);
}

uint16_t co2Samples[12];
uint16_t tempSamples[12];
uint16_t humiditySamples[12];
uint8_t sampleIdx = 0;
void loop()
{
  uint16_t error;
  char errorMessage[256];

  delay(5000);

  // Read Measurement
  uint16_t co2;
  uint16_t temperature;
  uint16_t humidity;
  error = scd4x.readMeasurementTicks(co2, temperature, humidity);
  if (error)
  {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else if (co2 == 0)
  {
    Serial.println("Invalid sample detected, skipping.");
  }
  else
  {
    Serial.print("Co2:");
    Serial.print(co2);
    Serial.print("\t");
    Serial.print("Temperature:");
    Serial.print(temperature);
    Serial.print("\t");
    Serial.print("Humidity:");
    Serial.println(humidity);
    encodeFrame(co2);
    matrix.renderBitmap(frame, 8, 12);
    sendSample(co2, temperature, humidity);
  }
}