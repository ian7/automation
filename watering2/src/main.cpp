#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "wifi-password.h"
#include "DHT.h"

#define LED_BUILTIN 5
#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char version[] = "30";
const char projectName[] = "watering2";

const int pinValve1 = 25;
const int pinValve2 = 17;
const int pinValve3 = 33;

const int pinMoisturePower = 26;

const int pinMoisture1 = 34;
const int pinMoisture2 = 32;
const int pinMoisture3 = 35;

WiFiClient net;
PubSubClient client(net);

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
  client.publish((char *)topicChars, (char *)payloadChars); //,false,1);
}

void messageReceived(String &topic, String &payload)
{
  if (topic == String("/watering2/pump1"))
  {
    int power = payload.toInt();
    ledcWrite(1, power);
    publish("/watering2/ack", "pump: " + payload);
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

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
  }

  while (!client.connect(projectName))
  {
    delay(200);
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
  publish("/watering2/version", version);

  client.subscribe("/watering2/+");

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
  int lastPingTime = millis();

  while (true)
  {
    delay(50);
    ArduinoOTA.handle();
    client.loop();
    if (!client.connected() || WiFi.status() != WL_CONNECTED)
    {
      connect();
    }

    int nowTime = millis();

/*    if ()
    {
      wifiReconnect();
    }
*/
    if (nowTime - lastPingTime > 10000)
    {
      lastPingTime = nowTime;
      //      if( WiFi.ping("10.10.1.4") < 0 ) {
      //       connect();
      //     }
    }
  }
}
