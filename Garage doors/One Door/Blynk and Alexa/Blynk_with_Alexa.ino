// Programme de domotique pour les garages de la maison - Julien Muggli, 2018
// Décommenter "#define BLYNK_PRINT Serial" pour debug
//#define BLYNK_PRINT Serial
#define BLYNK_MAX_SENDBYTES 256 // Augmentation du nombre de caractères pour les notifications
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h>      // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries (use the correct version)
#include <StreamString.h>

// BLYNK -------------------------------------------------------------------------------------------------------
char auth[] = "";
// Informations de connexion au WiFi
char ssid[] = "";
char pass[] = "";
// Diverses déclarations
const int power = LED_BUILTIN;
const int wifi = 2;
bool isFirstConnect = true;
WidgetLED led(V1);
#define BLYNK_GREEN "#23C48E"
#define BLYNK_RED "#D3435C"
#define BLYNK_YELLOW "#DDAD3B"
SimpleTimer timer;
// Variables will change:
unsigned long lastPress1 = 0;
unsigned long stateTime1 = 500; //runs until two seconds elapse

// Indique si le NodeMCU est connecté au Wi-Fi et au serveur
// (Utile une fois que l'on utilise plus le moniteur série de l'Arduino IDE)
// Pour les LED sur le NodeMCU, LOW = On et HIGH = Off
BLYNK_CONNECTED()
{
  if (isFirstConnect)
  {
    digitalWrite(wifi, LOW);
    isFirstConnect = false;
    Blynk.virtualWrite(V0, "Checking status...");
    led.setColor(BLYNK_YELLOW);
    Blynk.notify("Le garage de la Lexus est connecté !");
    digitalWrite(wifi, LOW);
    delay(1000);
    digitalWrite(power, HIGH);
    digitalWrite(wifi, HIGH);
    delay(100);
    digitalWrite(power, LOW);
    digitalWrite(wifi, LOW);
    delay(100);
    digitalWrite(power, HIGH);
    digitalWrite(wifi, HIGH);
    delay(100);
    digitalWrite(power, LOW);
    digitalWrite(wifi, LOW);
    delay(100);
    digitalWrite(power, HIGH);
    digitalWrite(wifi, HIGH);
    delay(100);
    digitalWrite(power, LOW);
    digitalWrite(wifi, LOW);
    delay(100);
    digitalWrite(power, HIGH);
    digitalWrite(wifi, HIGH);
    delay(100);
    digitalWrite(power, LOW);
    digitalWrite(wifi, LOW);
    delay(100);
    digitalWrite(power, HIGH);
    digitalWrite(wifi, HIGH);
    delay(100);
    digitalWrite(power, LOW);
    digitalWrite(wifi, LOW);
    delay(500);
    digitalWrite(power, HIGH);
    digitalWrite(wifi, HIGH);
  }
}

void relayGarageLexus()
{
  if (digitalRead(D5))
    lastPress1 = millis();
  if (millis() - lastPress1 < stateTime1)
  {
    digitalWrite(D7, LOW);
  }
  else
  {
    digitalWrite(D7, HIGH);
  }
}

void statutGarageLexus()
{
  if (digitalRead(D3))
  {
    led.setColor(BLYNK_RED); // Le reed switch est ouvert, D3 est HIGH
    Serial.println("Lexus ouvert");
    Blynk.virtualWrite(V0, "Garage Lexus ouvert");
  }
  else
  {
    led.setColor(BLYNK_GREEN); // Le reed switch est fermé, D3 est LOW
    Serial.println("Lexus fermé");
    Blynk.virtualWrite(V0, "Garage Lexus fermé");
  }
}

// SINRIC -------------------------------------------------------------------------------------------------------
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;
#define MyApiKey ""
#define MySSID ""
#define MyWifiPassword ""
#define DEVICE1 ""

void turnOn(String deviceId)
{
  if (deviceId == DEVICE1)
  {
    if (digitalRead(D3))
    {
      digitalWrite(D7, HIGH);
    }
    else
    {
      digitalWrite(D7, LOW);
      delay(500);
      digitalWrite(D7, HIGH);
    }
  }
}

void turnOff(String deviceId)
{
  if (deviceId == DEVICE1)
  {
    if (digitalRead(D3))
    {
      digitalWrite(D7, LOW);
      delay(500);
      digitalWrite(D7, HIGH);
    }
    else
    {
      digitalWrite(D7, HIGH);
    }
  }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    isConnected = false;
    Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
    break;
  case WStype_CONNECTED:
  {
    isConnected = true;
    Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
    Serial.printf("Waiting for commands from sinric.com ...\n");
  }
  break;
  case WStype_TEXT:
  {
    Serial.printf("[WSc] get text: %s\n", payload);
    // Example payloads

    // For Switch or Light device types
    // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

    // For Light device type
    // Look at the light example in github

    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject((char *)payload);
    String deviceId = json["deviceId"];
    String action = json["action"];

    if (action == "setPowerState")
    { // Switch or Light
      String value = json["value"];
      if (value == "ON")
      {
        turnOn(deviceId);
      }
      else
      {
        turnOff(deviceId);
      }
    }
    else if (action == "test")
    {
      Serial.println("[WSc] received test command from sinric.com");
    }
  }
  break;
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    break;
  }
}

// SETUP -------------------------------------------------------------------------------------------------------
void setup()
{
  // BLYNK -------------------------------------------------------------------------------------------------------
  pinMode(power, OUTPUT); // Déclaration LED alimentation
  pinMode(wifi, OUTPUT);  // Déclaration LED WiFi
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  digitalWrite(power, LOW);                                       // Déclaration LED alimentation est allumé
  digitalWrite(wifi, HIGH);                                       // Déclaration LES WiFi est éteinte
  digitalWrite(D7, HIGH);                                         // Relai en position Off si il y a un reboot
  pinMode(D7, OUTPUT);                                            // Connection au relai du garage de la Lexus
  pinMode(D3, INPUT_PULLUP);                                      // Connection au reed switch du garage de la Lexus
  Blynk.begin(auth, ssid, pass, IPAddress(192, 168, 0, 1), 8080); // Connection au WiFi
  // Allumage des LED, pour les voir dans l'app Blynk
  led.on();
  timer.setInterval(1500, statutGarageLexus);

  // SINRIC -------------------------------------------------------------------------------------------------------
  Serial.begin(115200);
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);
  // Waiting for Wifi connect
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  if (WiFiMulti.run() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");
  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000); // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

// LOOP -------------------------------------------------------------------------------------------------------
void loop()
{
  // BLYNK -------------------------------------------------------------------------------------------------------
  Blynk.run();
  timer.run();
  relayGarageLexus();
  if (WiFi.status() == 6) // Check la connexion. Si deconnecté, alors il se reset jusqu'à ce que la connexion soit à nouveau établie
  {
    ESP.reset();
  }

  // SINRIC -------------------------------------------------------------------------------------------------------
  webSocket.loop();
  if (isConnected)
  {
    uint64_t now = millis();

    // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
    if ((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL)
    {
      heartbeatTimestamp = now;
      webSocket.sendTXT("H");
    }
  }
}
