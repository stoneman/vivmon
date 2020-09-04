#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "Adafruit_VEML6075.h"
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "YOUR-WIFI-SSID"
#define STAPSK  "YOUR-WIFI-PASSWORD"
#endif

// The i2c pins for the VEML6075
const int UV_SDA = 0;
const int UV_SCL = 2;

// DHT22 temperature sensors
// First parameter is the sensors data pin
// DHT instances must be added to `dht_list` and require corresponding `JsonObject`s in `dht_json`
DHT dht_1(4, DHT22);
DHT dht_2(5, DHT22);
DHT dht_3(12, DHT22);
DHT dht_4(14, DHT22);

DHT dht_list[] = { dht_1, dht_2, dht_3, dht_4 };
int dht_count = sizeof(dht_list) / sizeof(DHT);

const size_t capacity = dht_count*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(dht_count+1);
DynamicJsonDocument doc(capacity);

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

JsonObject dht_json[] = { doc.createNestedObject("dht_1"), doc.createNestedObject("dht_2"), doc.createNestedObject("dht_3"), doc.createNestedObject("dht_4") };
JsonObject uv_json = doc.createNestedObject("uv");

Adafruit_VEML6075 uv = Adafruit_VEML6075();

void handleRoot() {
  for (int i = 0; i < dht_count; i++) {
    DHT dht = dht_list[i];
    float temp = dht.readTemperature();
    if (!isnan(temp)) {
      dht_json[i]["temperature"] = temp;
    }      
    float humidity = dht.readHumidity();
    if (!isnan(humidity)) {
      dht_json[i]["humidity"] = humidity;
    }
  }
  uv_json["uva"] = uv.readUVA();
  uv_json["uvb"] = uv.readUVB();
  uv_json["uvi"] = uv.readUVI();

  String message;
  serializeJsonPretty(doc, message);
  
  server.send(200, F("text/plain"), message);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
}

void setup(void) {
  Serial.begin(115200);
  
  for (int i = 0; i < dht_count; i++) {
    dht_list[i].begin();
  }

  Wire.begin(UV_SDA, UV_SCL);

  while (!uv.begin()) {
    Serial.println(F("Failed to communicate with VEML6075 sensor, check wiring?"));
    delay(500);
  }
  
  uv.setForcedMode(false);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println();
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  
  Serial.println();
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));

  server.on(F("/"), handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
