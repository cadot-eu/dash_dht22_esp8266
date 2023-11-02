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
#include <arduino_secrets.h>
#include <ESPDashPro.h>

#include <DHT.h>
#define DHTPIN 4  // Digital pin connected to the DHT sensor
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE DHT22  // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
float Temperature;
float Humidity;


/* Your SoftAP WiFi Credentials */
const char* ssidap = "AP";              // SSID
const char* passwordap = "ababababab";  // Password

const char* ssid = "wifi";            // SSID
const char* password = "ababababab";  // Password

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

Chart charttemp(&dashboard, LINE_CHART, "Tempétrature sur 72h");
Chart charthumidity(&dashboard, LINE_CHART, "Humidité sur 72h");

String XAxis[72];
float YAxisT[72];
float YAxisH[72];
int pointer = 0;
int boucle = 6 * 60;
void setup() {
  Serial.begin(115200);
charttemp.setSize({ .xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12 });
charthumidity.setSize({ .xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12 });
  //on initialise les tableaux

  charttemp.updateX(XAxis, 72);
  charthumidity.updateX(XAxis, 72);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pinMode(DHTPIN, INPUT);

  dht.begin();

  /* Start AsyncWebServer */
  server.begin();
  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  temps.update(int((6*600)-(boucle*10) ));
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    /* Start Access Point */
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssidap, passwordap);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
  Temperature = dht.readTemperature();  // Gets the values of the temperature
  Humidity = dht.readHumidity();        // Gets the values of the humidity
  /* Update Card Values */
  temperature.update(Temperature);
  humidity.update(Humidity);
  if (boucle >= (6 * 60))  //toutes les heures
  {
    boucle=0;
    if (pointer > (72 - 1)) {  //si on dépasse la limite du tableau on effectue un décalage
      for (int i = 0; i < (72 - 1); i++) {
        YAxisT[i] = YAxisT[i + 1];
        YAxisH[i] = YAxisH[i + 1];
      }
      YAxisT[72 - 1] = Temperature;
      YAxisH[72 - 1] = Humidity;
    } else {
      YAxisT[pointer] = Temperature;
      YAxisH[pointer] = Humidity;
    }
    charttemp.updateY(YAxisT, 72);
    charthumidity.updateY(YAxisH, 72);
    pointer++;
  }
  else
  {boucle++;}


  dashboard.sendUpdates();

  /*
    Delay is just for demonstration purposes in this example,
    Replace this code with 'millis interval' in your final project.
  */
  delay(10000);
}
