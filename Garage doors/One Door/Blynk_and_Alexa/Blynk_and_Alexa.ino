// Adaptation of my Blynk project with Alexa - Julien Muggli, 2019
// Uncomment "#define BLYNK_PRINT Serial" for debug
//#define BLYNK_PRINT Serial
#define BLYNK_MAX_SENDBYTES 256 // Raising notification character limit
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h>      // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries (use the correct version)
#include <StreamString.h>

// BLYNK -------------------------------------------------------------------------------------------------------
char auth[] = "YOUR_BLYNK_AUTH_CODE";
// Your WiFi information for Blynk
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
// Other declarations for Blynk
const int power = LED_BUILTIN;
const int wifi = 2;
bool isFirstConnect = true;
WidgetLED led(V1);
#define BLYNK_GREEN "#23C48E"
#define BLYNK_RED "#D3435C"
#define BLYNK_YELLOW "#DDAD3B"
SimpleTimer timer;
// Timer for Blynk, the second timer will define how long Blynk will hold the relay closed for you
unsigned long lastPress1 = 0;
unsigned long stateTime1 = 500;

// This small programm will notify you of the status of your NodeMCU.
// (Really useful once your project is installed in your garage and you don't have access to a serial monitor)
// It's blink the buil-in LED on the NodeMCU when connected to your WiFi and turn off after.
BLYNK_CONNECTED()
{
  if (isFirstConnect)
  {
    digitalWrite(wifi, LOW);
    isFirstConnect = false;
    Blynk.virtualWrite(V0, "Checking status...");
    led.setColor(BLYNK_YELLOW);
    // Please enable the notification widget in your Blynk project to get this notification
    Blynk.notify("Your garage is connected");
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
    led.setColor(BLYNK_RED); // Reed switch is open, D3 is HIGH
    Serial.println("Garage open");
    Blynk.virtualWrite(V0, "Garage door open");
  }
  else
  {
    led.setColor(BLYNK_GREEN); // Reed switch is closed, D3 is LOW
    Serial.println("Garage closed");
    Blynk.virtualWrite(V0, "Garage door closed");
  }
}

// SINRIC -------------------------------------------------------------------------------------------------------
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;
#define MyApiKey "YOUR_SINRIC_API_KEY"
#define MySSID "YOUR_WIFI_SSID"
#define MyWifiPassword "YOUR_WIFI_PASSWORD"
#define DEVICE1 "YOUR_DEVICE_ID_FROM_SINRIC"

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
  pinMode(power, OUTPUT); // Declaration for the built-in LED
  pinMode(wifi, OUTPUT);
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  digitalWrite(power, LOW);
  digitalWrite(wifi, HIGH);
  digitalWrite(D7, HIGH);
  pinMode(D7, OUTPUT);
  pinMode(D3, INPUT_PULLUP);
  Blynk.begin(auth, ssid, pass, IPAddress(192, 168, 0, 1), 8080); // Connection to your Blynk Server (if you have one)
  // If you don't have a Blynk Server, please comment the line above and uncomment the line below
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
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
  if (WiFi.status() == 6) // This is to check if the NodeMCU is still connected to your WiFi. If not, it'll reset itself and reboot until the connection is back
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
