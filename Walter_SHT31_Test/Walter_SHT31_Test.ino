/*
 * File: Walter_SHT31_Test.ino
 * 
 * Query Temperature and Humidity of the Environment by SHT31 connected to Walter-Module 
 *  
 * created by Claus Kühnel 2025-07-04 info@ckuehnel.ch
 */

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"

#define PWR_3V3_EN_PIN 0
#define I2C_SDA_PIN 42
#define I2C_SCL_PIN 2

bool enableHeater = false;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() 
{
  Serial.begin(115200);

  delay(5000); // wait for serial monitor

  Serial.println("Walter SHT31 Test");

  pinMode(PWR_3V3_EN_PIN, OUTPUT);
  pinMode(I2C_SDA_PIN, INPUT);
  pinMode(I2C_SCL_PIN, INPUT);


  /* Initialize I2C masters */
  digitalWrite(PWR_3V3_EN_PIN, LOW);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  delay(1000);
  
    if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
}

void loop() {
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t,1); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h,0);
  } else { 
    Serial.println("Failed to read humidity");
  }

  delay(1000);
}