/*
 * File: Walter_DS18B20_Test.ino
 * 
 * Query Temperature of the Environment by DS18B20 connected to Walter-Module 
 *  
 * created by Claus Kühnel 2025-07-04 info@ckuehnel.ch
 */
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin-Konfiguration
#define PWR_3V3_EN_PIN 0
#define ONE_WIRE_BUS 10  // DS18B20 Datenleitung an Pin GPIO10

// Objekte für DS18B20 und OLED
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


void setup() 
{
  // Serieller Monitor
  Serial.begin(115200);
  delay(5000);
  Serial.println("Walter DS18B20 Test");
  
  // Switched 3V3 on
  pinMode(PWR_3V3_EN_PIN, OUTPUT);
  digitalWrite(PWR_3V3_EN_PIN, LOW);

  // DS18B20 initialisieren
  sensors.begin();
  delay(1000);
}

void loop() 
{
  // Temperatur messen
  sensors.requestTemperatures();  
  float temperatureC = sensors.getTempCByIndex(0);

  // Temperatur auf dem seriellen Monitor ausgeben
  Serial.print(F("Temp = "));
  Serial.print(temperatureC);
  Serial.println(F(" °C"));


  // Kurze Pause
  delay(1000);
}