// Walter WiFi OpenWeather Station

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

char* ssid     = SECRET_SSID;
char* password = SECRET_PASS;
char* appid    = SECRET_APPID;

#define DEBUG 1  // Select debug mode by 1

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 2 minutes (120000)
//unsigned long timerDelay = 120000;  // 1000 calls/day free
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

StaticJsonDocument<1024> doc;
String jsonBuffer;

bool firstTime = true;

void setup() 
{
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("Walter WiFi Weather Station"));

  Serial.println(F("Initialized"));

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
	// Send an HTTP GET request
  if (((millis() - lastTime) > timerDelay) || firstTime) 
  {
    firstTime = false;

    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";
      //String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=Altendorf%20SZ,CH&appid=68f41be621dc61000324339afa4873d1&units=metric";
      Serial.print("\n\nCall to Openweather: ");
      Serial.println(serverPath);

      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);

      DeserializationError error = deserializeJson(doc, jsonBuffer);

      if (error) 
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        //return;
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
      Serial.printf("Wind Speed: %.1f m/s", windspeed);
    }
    else Serial.println("WiFi Disconnected");
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) 
{
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) 
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else 
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}