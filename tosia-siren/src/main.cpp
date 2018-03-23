#include <Arduino.h>
#include <MQTTClient.h>
#include <WiFi.h>
#include "wifi-password.h"

WiFiClient net;
MQTTClient client;

int lights[12];

int sirenMode = 0;
int sirenStep = 0;
int sirenDelay = 100;
int signalLength = 100;
int signalDelay = 400;
int signalCount = 0;

// this is defined in wifi-password.h
//const char ssid[] = "xxx";
//const char pass[] = "xxx";
void messageReceived(String &topic, String &payload)
{
  if (topic == String("/tosia/siren/mode"))
  {
    sirenMode = payload.toInt();
    client.publish("/tosia/siren/ack", "mode: " + payload);
  }

  if (topic == String("/tosia/siren/delay"))
  {
    sirenDelay = payload.toInt();
    client.publish("/tosia/siren/ack", "delay: " + payload);
  }

  if (topic == String("/tosia/signal/length"))
  {
    signalLength = payload.toInt();
    client.publish("/tosia/siren/ack", "signalLength: " + payload);
  }

  if (topic == String("/tosia/signal/delay"))
  {
    signalDelay = payload.toInt();
    client.publish("/tosia/siren/ack", "signalDelay: " + payload);
  }

  if (topic == String("/tosia/signal/count"))
  {
    signalCount = payload.toInt();
    client.publish("/tosia/siren/ack", "signalCount: " + payload);
  }
}

void connect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  while (!client.connect("tosia-siren"))
  {
    delay(1000);
  }
}

void setup()
{
  lights[0] = 23;
  lights[1] = 22;
  lights[2] = 21;
  lights[3] = 19;
  lights[4] = 18;
  //lights[5] = 13;
  lights[5] = 26;
  lights[6] = 27;
  lights[7] = 14;
  lights[8] = 12;
  lights[9] = 05;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  client.begin("10.10.4.1", net);
  client.onMessage(messageReceived);
  connect();

  for (int i = 0; i < 11; i++)
  {
    pinMode(lights[i], OUTPUT);
  }
  pinMode(17, OUTPUT);

  client.subscribe("/tosia/siren/mode");
  client.subscribe("/tosia/siren/delay");
  client.subscribe("/tosia/signal/length");
  client.subscribe("/tosia/signal/delay");
  client.subscribe("/tosia/signal/count");
}

void signalOn()
{
  digitalWrite(17, HIGH);
}

void signalOff()
{
  digitalWrite(17, LOW);
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
      if( sirenStep >= 10) {
        sirenStep = 0;
      }
      delay(sirenDelay);
      break;
    case 2:
      allOff();
      on(sirenStep);
      on(sirenStep + 5);
      sirenStep = (sirenStep + 1);
      if( sirenStep >= 5) {
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
    if( signalCount > 0 ){
      signalOn();
      delay( signalLength );
      signalOff();
      delay( signalDelay );
      signalCount--;
    }
  }
}