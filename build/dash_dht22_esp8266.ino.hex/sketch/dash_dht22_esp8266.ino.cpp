#line 1 "/home/michael/git/arduino/dash_dht22_esp8266/dash_dht22_esp8266.ino"
#include <Arduino.h>
#if defined(ESP8266)
/* ESP8266 Dependencies */
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#elif defined(ESP32)
/* ESP32 Dependencies */
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif
#include <ESPDashPro.h>

#include <DHT.h>
#define DHTPIN 4 // Digital pin connected to the DHT sensor
// #define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
// #define DHTTYPE    DHT21     // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
float Temperature;
float Humidity;

/* Your SoftAP WiFi Credentials */
#include "arduino_secrets.h"
const char *ssidap = SECRET_SSID_AP;     // SSID
const char *passwordap = SECRET_PASS_AP; // Password

const char *ssid = SECRET_SSID;     // SSID
const char *password = SECRET_PASS; // Password

/* Start Webserver */
AsyncWebServer server(80);

/* Attach ESP-DASH to AsyncWebServer */
ESPDash dashboard(&server);

/*
  Dashboard Cards
  Format - (Dashboard Instance, Card Type, Card Name, Card Symbol(optional) )
*/
Card temperature(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
Card humidity(&dashboard, HUMIDITY_CARD, "Humidity", "%");
Card temps(&dashboard, GENERIC_CARD, "temps avant mesure", "s");

Chart charttemp(&dashboard, LINE_CHART, "Tempétrature sur 96h");
Chart charthumidity(&dashboard, LINE_CHART, "Humidité sur 96h");

String XAxis[96];
float YAxisT[96];
float YAxisH[96];
int pointer = 0;
int boucle = 6 * 60;
#line 54 "/home/michael/git/arduino/dash_dht22_esp8266/dash_dht22_esp8266.ino"
void setup();
#line 88 "/home/michael/git/arduino/dash_dht22_esp8266/dash_dht22_esp8266.ino"
void loop();
#line 54 "/home/michael/git/arduino/dash_dht22_esp8266/dash_dht22_esp8266.ino"
void setup()
{
  Serial.begin(115200);
  charttemp.setSize({.xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12});
  charthumidity.setSize({.xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12});
  // on initialise les tableaux
  for (int i = 0; i < (96 - 1); i++)
  {
    XAxis[i] = i;
  }

  charttemp.updateX(XAxis, 96);
  charthumidity.updateX(XAxis, 96);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pinMode(DHTPIN, INPUT);

  dht.begin();

  /* Start AsyncWebServer */
  server.begin();
  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  temps.update(int((6 * 600) - (boucle * 10)));
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    /* Start Access Point */
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssidap, passwordap);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity();       // Gets the values of the humidity
  /* Update Card Values */
  temperature.update(Temperature);
  humidity.update(Humidity);
  if (boucle >= (6 * 60)) // toutes les heures
  {
    boucle = 0;
    if (pointer > (96 - 1))
    { // si on dépasse la limite du tableau on effectue un décalage
      for (int i = 0; i < (96 - 1); i++)
      {
        YAxisT[i] = YAxisT[i + 1];
        YAxisH[i] = YAxisH[i + 1];
      }
      YAxisT[96 - 1] = Temperature;
      YAxisH[96 - 1] = Humidity;
    }
    else
    {
      YAxisT[pointer] = Temperature;
      YAxisH[pointer] = Humidity;
    }
    charttemp.updateY(YAxisT, 96);
    charthumidity.updateY(YAxisH, 96);
    pointer++;
  }
  else
  {
    boucle++;
  }

  dashboard.sendUpdates();

  /*
    Delay is just for demonstration purposes in this example,
    Replace this code with 'millis interval' in your final project.
  */
  delay(10000);
}

