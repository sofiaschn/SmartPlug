#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

#define OUTPUT_LEN 3
#define GPIO12 12
#define GPIO13 13
#define GPIO14 14

#define AP_SSID "SmartPlug"

#define OTA_HOSTNAME "SmartPlugOTA"
#define OTA_PASSWORD "SmartPlugOTA"

ESP8266WebServer server(80);

const int outputs[OUTPUT_LEN] = [GPIO12, GPIO13, GPIO14];

void handleRoot(void);
void handleGPIO(int gpio);

void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    Serial.println("[*] OTA: Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[*] OTA: End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[*] OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[!] OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("[!] OTA: Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("[!] OTA: Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("[!] OTA: Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("[!] OTA: Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("[!] OTA: End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("[*] OTA setup finished.");
}

void setup() {
  Serial.begin(115200);
  Serial.println("[*] Starting device.", true);

  for (int i = 0; i < OUTPUT_LEN; i++) {
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], LOW);
  }

  WiFi.setSleepMode(WIFI_MODEM_SLEEP, 10);
  WiFi.setOutputPower(0);
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  
  WiFiManager manager;
  manager.autoConnect(AP_SSID);
  
  Serial.print("[*] Connected to ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP address: ")
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);

  for (int i = 0; i < OUTPUT_LEN; i++) {
    String gpio = "/gpio/";
    String path =  gpio + outputs[i];
    server.on(path, HTTP_POST, [](){
      handleGPIO(outputs[i]);
    });
  }

  server.onNotFound([](){
    server.send(404, "text/plain", "404: Not found");
  });

  server.begin();
  Serial.println("HTTP server started");

  setupOTA();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Toggle LED\"></form>");
}

void handleGPIO(int gpio) {

}
