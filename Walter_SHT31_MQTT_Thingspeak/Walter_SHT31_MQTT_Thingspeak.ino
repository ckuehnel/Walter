/*
 * File: Walter_SHT31_MQTT_Thingspeak.ino
 * 
 * Walter LTE Weather Station
 * Send Environmental Data measured by SHT31 via MQTT to Thingspeak 
 *  
 * created by Claus Kühnel 2025-07-04 info@ckuehnel.ch
 * based on mqtt.ino by DPTechnics bv
 *
 * Arduino IDE 2.3.10 compiled @ 2026-06-20
 */

/**
 * @file mqtt.ino
 * @author Jonas Maes <jonas@dptechnics.com>
 * @date 28 Apr 2025
 * @copyright DPTechnics bv
 * @brief Walter Modem library examples
 *
 * @section LICENSE
 *
 * Copyright (C) 2025, DPTechnics bv
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of DPTechnics bv nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *   4. This software, with or without modification, must only be used with a
 *      Walter board from DPTechnics bv.
 *
 *   5. Any software provided in binary form under this license must not be
 *      reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY DPTECHNICS BV “AS IS” AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL DPTECHNICS BV OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 *
 * This file contains a sketch which uses the modem in Walter to make a
 * http request and show the result.
 */

#include <Arduino.h>
#include <HardwareSerial.h>
#include <WalterModem.h>      // v1.5.0
#include <Wire.h>
#include "Adafruit_SHT31.h"   // v2.2.2
#include "mqtt_secrets.h"

// Pins for SHT31 sensor
#define PWR_3V3_EN_PIN 0
#define I2C_SDA_PIN 42
#define I2C_SCL_PIN 2

bool enableHeater = false;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

const char* mqttUser = SECRET_MQTT_USERNAME;
const char* mqttPass = SECRET_MQTT_PASSWORD;
const char* mqttId   = SECRET_MQTT_CLIENT_ID;

const char* topic = "channels/3003328/publish"; 

/**
 * @brief The modem instance.
 */
WalterModem modem;

/**
 * @brief Response object containing command response information.
 */
WalterModemRsp rsp;

/**
 * @brief Buffer for incoming response
 */
uint8_t incomingBuf[256] = {0};

void setup() 
{
  Serial.begin(115200);
  delay(5000);

  Serial.println("Walter sends SHT31 data via MQTT to Thingspeak MQTT Broker v1.0.0\r\n");

  // Configure IO
  pinMode(PWR_3V3_EN_PIN, OUTPUT);
  pinMode(I2C_SDA_PIN, INPUT);
  pinMode(I2C_SCL_PIN, INPUT);

  /* Initialize I2C master */
  digitalWrite(PWR_3V3_EN_PIN, LOW);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  delay(1000);
  
    if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  Serial.print("SHT31 Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");

  if (WalterModem::begin(&Serial2)) {
    Serial.println("Modem initialization OK");
  } else {
    Serial.println("Error: Modem initialization ERROR");
    return;
  }
  /* Connect the modem to the lte network */
  if (lteConnect()) {
    Serial.println("Connected to LTE");
  } else {
    Serial.println("Error: Could Not Connect to LTE");
    return;
  }

  /* configure the mqtt client */
  if (modem.mqttConfig(mqttId, mqttUser, mqttPass)) {
    Serial.println("MQTT configuration succeeded");
  } else {
    Serial.println("Error: MQTT configuration failed");
    return;
  }

  // Thingspeak MQTT broker 
  if(modem.mqttConnect("mqtt3.thingspeak.com", 1883))
  {
    Serial.println("MQTT connection succeeded");
  } else {
    Serial.println("Error: MQTT connection failed");
  }
}

void loop() 
{
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (isnan(t)) {  // check if 'is not a number'
    Serial.println("Failed to read temperature");
  }
  
  if (isnan(h)) {  // check if 'is not a number' 
    Serial.println("Failed to read humidity");
  }

  Serial.printf("SHT31 Temperature %0.1f°C\tHumidity %0.0f %%rH\n", t,h);
 
  /* Publish data to ThingsSpeak over MQTT */
  static char outgoingMsg[64] = { 0 };
  sprintf(outgoingMsg, "field1=%.1f&field2=%.0f", t, h);
  Serial.printf("Going to publish '%s' to ThingSpeak\n", outgoingMsg);

  if(modem.mqttPublish(topic, (uint8_t*) outgoingMsg, strlen(outgoingMsg), 0)) 
  {
    Serial.printf("Published to ThingSpeak on topic %s\n", topic);
  } else {
    Serial.print("Could not publish to ThingSpeak\n");
  }

  delay(10000);
}
