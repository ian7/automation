#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "wifi-password.h"

WiFiClient net;
PubSubClient client(net);

int lights[12];

int sirenMode = 0;
int sirenStep = 0;
int sirenDelay = 100;
int signalLength = 100;
int signalDelay = 400;
int signalCount = 0;
int signalPower = 50;
bool isSignalOn = false;
int signalSwitchTime = 0;

// this is defined in wifi-password.h
//const char ssid[] = "xxx";
//const char pass[] = "xxx";

void publish(const String &topic, const String &payload)
{
  unsigned char topicChars[100];
  unsigned char payloadChars[100];
  topic.getBytes(topicChars, 100);
  payload.getBytes(payloadChars, 100);
  client.publish((char *)topicChars, (char *)payloadChars); //,false,1);
}

void messageReceived(String &topic, String &payload)
{
  if (topic == String("/tosia/siren/mode"))
  {
    sirenMode = payload.toInt();
    publish("/tosia/siren/ack", "mode: " + payload);
  }

  if (topic == String("/tosia/siren/delay"))
  {
    sirenDelay = payload.toInt();
    publish("/tosia/siren/ack", "delay: " + payload);
  }

  if (topic == String("/tosia/signal/length"))
  {
    signalLength = payload.toInt();
    publish("/tosia/siren/ack", "signalLength: " + payload);
  }

  if (topic == String("/tosia/signal/delay"))
  {
    signalDelay = payload.toInt();
    publish("/tosia/siren/ack", "signalDelay: " + payload);
  }

  if (topic == String("/tosia/signal/count"))
  {
    signalCount = payload.toInt();
    publish("/tosia/siren/ack", "signalCount: " + payload);
  }

  if (topic == String("/tosia/signal/power"))
  {
    signalPower = payload.toInt();
    publish("/tosia/siren/ack", "signalPower: " + payload);
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
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
  }

  while (!client.connect("tosia-siren"))
  {
    delay(200);
  }
}

void signalOn()
{
  //digitalWrite(17, HIGH);
  ledcWrite(1, signalPower);
  isSignalOn = true;
}

void signalOff()
{
  //digitalWrite(17, LOW);
  ledcWrite(1, 0);
  isSignalOn = false;
}

void setup()
{
  lights[0] = 23;
  lights[1] = 22;
  lights[2] = 21;
  lights[3] = 19;
  lights[4] = 18;
  lights[5] = 26;
  lights[6] = 27;
  lights[7] = 14;
  lights[8] = 12;
  lights[9] = 05;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  client.setServer("10.10.4.1", 1883);
  client.setCallback(pubSubCallback);
  connect();

  for (int i = 0; i < 11; i++)
  {
    pinMode(lights[i], OUTPUT);
  }
  pinMode(17, OUTPUT);

  client.subscribe("/tosia/siren/+");
  client.subscribe("/tosia/signal/+");
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
  MDNS.begin("tosia-siren");
  publish("/tosia-siren/IP", WiFi.localIP().toString());
  publish("/tosia-siren/version", "8");

  ledcAttachPin(17, 1);
  ledcSetup(1, 100, 8);
  signalOff();
}

void allOn()
{
  for (int i = 0; i < 11; i++)
  {
    digitalWrite(lights[i], HIGH);
  }
}

void allOff()
{
  for (int i = 0; i < 11; i++)
  {
    digitalWrite(lights[i], LOW);
  }
}

void off(int i)
{
  digitalWrite(lights[i], LOW);
}

void on(int i)
{
  digitalWrite(lights[i], HIGH);
}

void loop()
{

  while (true)
  {
    delay(50);
    ArduinoOTA.handle();
    client.loop();
    if (!client.connected())
    {
      connect();
    }

    switch (sirenMode)
    {
    case 1:
      allOff();
      on(sirenStep);
      sirenStep = (sirenStep + 1);
      if (sirenStep >= 10)
      {
        sirenStep = 0;
      }
      delay(sirenDelay);
      break;
    case 2:
      allOff();
      on(sirenStep);
      on(sirenStep + 5);
      sirenStep = (sirenStep + 1);
      if (sirenStep >= 5)
      {
        sirenStep = 0;
      }
      delay(sirenDelay);
      break;
    case 3:
      if (sirenStep > 0)
      {
        allOff();
        sirenStep = 0;
      }
      else
      {
        allOn();
        sirenStep = 1;
      }
      delay(sirenDelay);
      break;
    case 10:
      allOff();
      on(0);
      break;
    case 11:
      allOff();
      on(1);
      break;
    case 12:
      allOff();
      on(2);
      break;
    case 13:
      allOff();
      on(3);
      break;
    case 14:
      allOff();
      on(4);
      break;
    case 15:
      allOff();
      on(5);
      break;
    case 16:
      allOff();
      on(6);
      break;
    case 17:
      allOff();
      on(7);
      break;
    case 18:
      allOff();
      on(8);
      break;
    case 19:
      allOff();
      on(9);
      break;
    case 20:
      allOff();
      on(10);
      break;
    default:
    case 0:
      allOff();
      delay(100);
      break;
    }

    int timeNow = millis();
    if (timeNow >= signalSwitchTime &&
        signalCount > 0)
    {

      if (isSignalOn == false)
      {
        signalOn();
        signalSwitchTime = timeNow + signalLength;
      }
      else
      {
        signalOff();
        signalSwitchTime = timeNow + signalDelay;
        signalCount--;
      }
    }
  }
}