#include "Arduino.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "wifi-password.h"
#include "DHT.h"
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>


#define LED_BUILTIN 5
#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char version[] = "40";
const char projectName[] = "watering2";

const int pinValve1 = 25;
const int pinValve2 = 17;
const int pinValve3 = 33;

const int pinMoisturePower = 26;

const int pinMoisture1 = 34;
const int pinMoisture2 = 32;
const int pinMoisture3 = 35;

const int pinFlow = 18;
int flowCounter = 0;

unsigned long wateringEnd = 0;
unsigned long lastHeartBeat = 0;
const unsigned long heartBeatInterval = 10000; // 10 seconds
const int reconnectCounterThreshold = 100; // retires happen every 200ms, so this gives 20s

WiFiClient net;
PubSubClient client(net);

void hard_restart() {
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}


void WiFiEvent(WiFiEvent_t event)
{
  if (event == SYSTEM_EVENT_STA_DISCONNECTED)
  {
    WiFi.reconnect();
  }
}

void publish(const String &topic, const String &payload)
{
  unsigned char topicChars[100];
  unsigned char payloadChars[100];
  topic.getBytes(topicChars, 100);
  payload.getBytes(payloadChars, 100);
  client.publish((char *)topicChars, (char *)payloadChars);
}

void closeAllValves()
{
  digitalWrite(pinValve1, LOW);
  digitalWrite(pinValve2, LOW);
  digitalWrite(pinValve3, LOW);
}

void pumpSetPower(int power)
{
  if( power > 0 ){
    flowCounter = 0;
  }
  ledcWrite(1, power);
}

void doWatering(int pin, int amount)
{
  publish("/watering2/ack", "watering: " + String(pin) + " for: " + String(amount) + " seconds");

  wateringEnd = millis() + amount * 1000;
  closeAllValves();
  digitalWrite(pin, HIGH);
  pumpSetPower(255);
}

void messageReceived(String &topic, String &payload)
{
  if (topic == String("/heartbeat")){
    lastHeartBeat = millis();
  }

  if (topic == String("/watering2/restart")){
    hard_restart();
  }

  if (topic == String("/watering2/pump1"))
  {
    int power = payload.toInt();
    ledcWrite(1, power);
    publish("/watering2/ack", "pump: " + payload);
  }
  if (topic == String("/watering2/water1"))
  {
    doWatering(pinValve1, payload.toInt());
  }
  if (topic == String("/watering2/water2"))
  {
    doWatering(pinValve2, payload.toInt());
  }
  if (topic == String("/watering2/water3"))
  {
    doWatering(pinValve3, payload.toInt());
  }
  if (topic == String("/watering2/millis"))
  {
    publish("/watering2/ack", "millis: " + String(millis()));
  }

  if (topic == String("/watering2/valve1"))
  {
    int power = payload.toInt();
    if (power == 0)
    {
      digitalWrite(pinValve1, LOW);
    }
    else
    {
      digitalWrite(pinValve1, HIGH);
    }
    publish("/watering2/ack", "valve1: " + payload);
  }
  if (topic == String("/watering2/valve2"))
  {
    int power = payload.toInt();
    if (power == 0)
    {
      digitalWrite(pinValve2, LOW);
    }
    else
    {
      digitalWrite(pinValve2, HIGH);
    }
    publish("/watering2/ack", "valve2: " + payload);
  }
  if (topic == String("/watering2/valve3"))
  {
    int power = payload.toInt();
    if (power == 0)
    {
      digitalWrite(pinValve3, LOW);
    }
    else
    {
      digitalWrite(pinValve3, HIGH);
    }
    publish("/watering2/ack", "valve3: " + payload);
  }
  if (topic == String("/watering2/moisturePoll"))
  {
    publish("/watering2/ack", "moisturePoll");

    digitalWrite(pinMoisturePower, LOW);
    delay(1000);

    int moisture1 = analogRead(pinMoisture1);
    int moisture2 = analogRead(pinMoisture2);
    int moisture3 = analogRead(pinMoisture3);

    digitalWrite(pinMoisturePower, HIGH);

    publish("/watering2/moisture1", String(moisture1));
    publish("/watering2/moisture2", String(moisture2));
    publish("/watering2/moisture3", String(moisture3));
  }

  if (topic == String("/watering2/temperaturePoll"))
  {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    publish("/watering2/temperature", String(t));
    publish("/watering2/humidity", String(h));
  }
}

void pubSubCallback(char *topic, byte *payload, unsigned int length)
{
  String payloadString = String((char *)payload).substring(0, length);
  String topicString = String(topic);
  messageReceived(topicString, payloadString);
}

void connect()
{
  client.disconnect();
  WiFi.disconnect(true);
  WiFi.setAutoReconnect(true);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  //WiFi.onEvent(WiFiEvent);

  client.setServer("10.10.4.1", 1883);
  client.setCallback(pubSubCallback);

  int reconnectCounter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    reconnectCounter++;
    if(reconnectCounter > reconnectCounterThreshold){
      hard_restart();
    }
  }

  reconnectCounter=0;
  while (!client.connect(projectName))
  {
    delay(200);
    reconnectCounter++;
    if(reconnectCounter > reconnectCounterThreshold){
      hard_restart();
    }
  }
}

void setup()
{
  dht.begin();
  connect();

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
  MDNS.begin(projectName);
  publish("/watering2/IP", WiFi.localIP().toString());
  publish("/watering2/RSSI", String(WiFi.RSSI()));
  publish("/watering2/version", version);

  client.subscribe("/watering2/+");
  client.subscribe("/heartbeat");

  ledcAttachPin(27, 1);
  ledcSetup(1, 100, 8);
  ledcWrite(1, 0);

  //valves:
  pinMode(pinValve1, OUTPUT);
  digitalWrite(pinValve1, LOW);
  pinMode(pinValve2, OUTPUT);
  digitalWrite(pinValve2, LOW);
  pinMode(pinValve3, OUTPUT);
  digitalWrite(pinValve3, LOW);

  pinMode(pinMoisturePower, OUTPUT);
  digitalWrite(pinMoisturePower, HIGH);

  pinMode(pinMoisture1, INPUT);
  digitalWrite(pinMoisture2, LOW);
  pinMode(pinMoisture2, INPUT_PULLDOWN);
  digitalWrite(pinMoisture3, LOW);
  pinMode(pinMoisture3, INPUT_PULLDOWN);
    
  lastHeartBeat = millis();
}

void wifiReconnect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, pass);
    for (int i = 0; i <= 50; i++)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      Serial.print(".");
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
}

void loop()
{
  unsigned long timeNow = 0;
  bool lastFlowState = false;

  while (true)
  {
    timeNow = millis();

    delay(50);
    ArduinoOTA.handle();
    client.loop();

    if (wateringEnd != 0)
    {
      if (timeNow > wateringEnd)
      {
        publish("/watering2/ack", "watering done at: " + String(timeNow) + " flow: " + String(flowCounter));

        pumpSetPower(0);
        closeAllValves();
        wateringEnd = 0;
        flowCounter = 0;
      }
      else
      {
        // here we are going to count pulses
        bool currentFlowState = digitalRead(pinFlow);
        if( currentFlowState != lastFlowState ){
          flowCounter++;
//          publish("/watering2/ack", "flow count at: " + String(flowCounter));
          lastFlowState = currentFlowState;
        }
      }
    }

    if (!client.connected() || WiFi.status() != WL_CONNECTED)
    {
      connect();
    }
      unsigned long timeNow = millis();
    if( lastHeartBeat != 0 && timeNow - lastHeartBeat > heartBeatInterval ){
      client.publish("/light/bedroom/watchdog","triggered");
      lastHeartBeat = 0;
      hard_restart();
    }
  }
}
