#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Adafruit_VEML6075.h"

char ssid[] = "YOUR-WIFI-SSID";
char pass[] = "YOUR-WIFI-PASSWORD";

WiFiServer server(80);

// DHT2 temperature sensors
// First parameter is the sensors data pin
DHT dht_1(4, DHT22);
DHT dht_2(5, DHT22);
DHT dht_3(12, DHT22);
DHT dht_4(14, DHT22);

DHT dht_list[] = { dht_1, dht_2, dht_3, dht_4 }; 
String dht_name_list[] =  { "dht_1", "dht_2", "dht_3", "dht_4"  };

int dht_count = sizeof(dht_list) / sizeof(DHT);

Adafruit_VEML6075 uv = Adafruit_VEML6075();

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  Serial.println();
  Serial.println();

  for (int i = 0; i < dht_count; i++) {
    dht_list[i].begin();
  }

  // Parameters are the VEML6075's SDA and SCL pins
  Wire.begin(0, 2);

  while (!uv.begin()) {
    Serial.println("Failed to communicate with VEML6075 sensor, check wiring?");
    delay(500);
  }
  
  uv.setIntegrationTime(VEML6075_800MS);
  uv.setHighDynamic(true);
  uv.setForcedMode(false);
    
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  
  server.begin(); 
  
  printWifiStatus();
}


void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          client.println("Refresh: 2");
          client.println();
          client.println("{");
          for (int i = 0; i < dht_count; i++) {
            DHT dht = dht_list[i];
            float temp = dht.readTemperature();
            float humidity = dht.readHumidity();
            if (isnan(temp) || isnan(humidity)) {
              continue;
            }
            String dht_name = dht_name_list[i];
            client.println("  \"" + dht_name + "\": {");
            client.print("    \"temperature\": ");
            client.print(temp);
            client.println(",");
            client.print("    \"humidity\": ");
            client.println(humidity);
            client.println("  },");
          }
          client.println("  \"uv\": {");
          client.print("    \"uva\": ");
          client.print(uv.readUVA());
          client.println(",");
          client.print("    \"uvb\": ");
          client.print(uv.readUVB());
          client.println(",");
          client.print("    \"uvi\": ");
          client.println(uv.readUVI());
          client.println("  }");
          client.println("}");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("client disonnected");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}
