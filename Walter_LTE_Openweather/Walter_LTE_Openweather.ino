/*
 * File: Walter_LTE_Openweather.ino
 * 
 * Walter LTE OpenWeather Station
 * Query Environmental Data from Openweather by LTE Access 
 *  
 * created by Claus Kühnel 2025-07-04 info@ckuehnel.ch
 * based on http_querry.ino by DPTechnics
 *
 * Arduino IDE 2.3.10 compiled @ 2026-06-20
 */

/**
 * @file http_querry.ino
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

#include <esp_mac.h>
#include <WalterModem.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>

/**
 * @brief HTTP profile
 */
#define MODEM_HTTP_PROFILE 1

/**
 * @brief TLS profile
 */
#define TLS_PROFILE 1

#define API_KEY "68f41be621dc61000324339afa4873d1"
#define CITY "Altendorf%20SZ"
#define COUNTRY_CODE "CH"

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 2 minutes (120000)
unsigned long timerDelay = 120000;  // 1000 calls/day free
// Set timer to 10 seconds (10000)
// unsigned long timerDelay = 10000;

StaticJsonDocument<1024> doc;
String jsonBuffer;

bool firstTime = true;

/**
 * @brief The modem instance.
 */
WalterModem modem;

/**
 * @brief Response object containing command response information.
 */
WalterModemRsp rsp = {};

/**
 * @brief Buffer for incoming HTTP response
 */
uint8_t incomingBuf[2048] = { 0 };

char argument[256]; // Enough space for full URL

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println(F("Walter LTE Weather Station"));
  
  if(WalterModem::begin(&Serial2)) {
    Serial.println("Modem initialization OK");
  } else {
    Serial.println("Error: Modem initialization ERROR");
    return;
  }
  modem.setRadioBands(WALTER_MODEM_RAT_LTEM, WALTER_MODEM_BAND_B20);
  /* Connect the modem to the lte network */
  if (!lteConnect()) {
    Serial.println("Error: Could Not Connect to LTE");
    return;
  }

  /* Connect the modem to the lte network */
  if (!lteConnect()) {
    Serial.println("Error: Could Not Connect to LTE");
    return;
  }

  if(modem.tlsConfigProfile(TLS_PROFILE)){
    Serial.println("Successfully configured the tlsProfile profile");
  } else {
    Serial.println("Error: Failed to configure HTTP profile");
  }

  /* Configure http profile for a simple test */
  if(modem.httpConfigProfile(MODEM_HTTP_PROFILE, "api.openweathermap.org", 443,TLS_PROFILE)) {
    Serial.println("Successfully configured the http profile");
  } else {
    Serial.println("Error: Failed to configure HTTP profile");
  }
}

void loop() {
  /* HTTP test */
  static short httpReceiveAttemptsLeft = 0;
  static char ctbuf[32];

  snprintf(argument, sizeof(argument),
          "/data/2.5/weather?q=%s,%s&appid=%s&units=metric",
          CITY, COUNTRY_CODE, API_KEY);

  Serial.printf("Call to Openweather: http://api.openweathermap.org/data/2.5/weather?q=%s,%s&appid=%s&units=metric\n",
          CITY, COUNTRY_CODE, API_KEY);

  if(!httpReceiveAttemptsLeft) {
    if(modem.httpQuery(MODEM_HTTP_PROFILE, argument, WALTER_MODEM_HTTP_QUERY_CMD_GET, ctbuf, sizeof(ctbuf))) {
      Serial.println("http query performed");
      httpReceiveAttemptsLeft = 3;
    } else {
      Serial.println("Error: http query failed");
      delay(1000);
      ESP.restart();
    }
  } else {
    /* while loop so we can fetch old responses as well as the expected response */
    while(modem.httpDidRing(MODEM_HTTP_PROFILE, incomingBuf, sizeof(incomingBuf), &rsp)) {
      httpReceiveAttemptsLeft = 0;

      Serial.printf("http status code: %d\r\n", rsp.data.httpResponse.httpStatus);
      Serial.printf("content type: %s\r\n", ctbuf);
      Serial.printf("[%s]\r\n", incomingBuf);

      DeserializationError error = deserializeJson(doc, incomingBuf);

      if (error) 
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }

      JsonObject main = doc["main"];
      float temp = main["temp"]; // 14.72
      int press = main["pressure"]; // 1012
      int humi = main["humidity"]; // 66

      JsonObject wind = doc["wind"];
      float windspeed = wind["speed"]; // 2.71

      Serial.printf("\nTemperature: %.1f °C\n", temp);
      Serial.printf("Pressure:  %d hPa\n", press); 
      Serial.printf("Humidity: %d %%rH\n", humi); 
      Serial.printf("Wind Speed: %.1f m/s\n\n", windspeed);
    }

    if(httpReceiveAttemptsLeft) {
      Serial.println("HTTP response not yet received\r\n");
      httpReceiveAttemptsLeft--;
    }
  }

  delay(10000);
}
