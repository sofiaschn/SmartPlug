#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h> 

#define OUTPUT_LEN 3
#define GPIO12 12
#define GPIO13 13
#define GPIO14 14

#define AP_SSID "SmartPlug"

#define OTA_HOSTNAME "SmartPlugOTA"
#define OTA_PASSWORD "SmartPlugOTA"

ESP8266WebServer server(80);

const int outputs[OUTPUT_LEN] = { GPIO12, GPIO13, GPIO14 };

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
  Serial.println("[*] Starting device.");

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
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);

  server.on("/gpio/12", HTTP_POST, [](){
      handleGPIO(GPIO12);
  });
  server.on("/gpio/13", HTTP_POST, [](){
      handleGPIO(GPIO13);
  });
  server.on("/gpio/14", HTTP_POST, [](){
      handleGPIO(GPIO14);
  });
  
  server.onNotFound([](){
    server.send(404, "text/plain", "404: Not found");
  });

  server.begin();
  Serial.println("[*] HTTP server started");

  setupOTA();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String response = "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                    "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
                    ".input { background-color: #195B6A; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
                    ".inputOff {background-color: #77878A;}</style></head>"
                    "<body><h1>Painel de Controle SmartPlug</h1>";

  for (int i = 0; i < OUTPUT_LEN; i++) { 
    int outputState = digitalRead(outputs[i]);

    String action = (outputState ? "Desligar Tomada " : "Ligar Tomada ");
    action += i + 1;

    response += "<form action=\"/gpio/" + String(outputs[i]) + "\" method=\"POST\">";

    String cssClass = !outputState ? "inputOff" : "";
    response += "<input class=\"input " + cssClass + "\" type=\"submit\" value=\"" + action + "\"></form>";
  }
  response += "</body>";
  server.send(200, "text/html", response);
}

void handleGPIO(int gpio) {
  Serial.printf("[*] Toggling GPIO %d, current state: %d\n", gpio, digitalRead(gpio));
  digitalWrite(gpio, !digitalRead(gpio));
  server.sendHeader("Location","/");
  server.send(303);
}
