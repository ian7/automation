
#define _GLIBCXX_USE_C99 1

#ifdef ESP8266
#include <Arduino.h>
#endif

#define LED_PIN 2

#define LED_ON 0
#define LED_OFF 1

#define SLEEP_PIN 0

#define ON true
#define OFF false

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "wifi-password.h"

#include <string>

#include <DHT.h>

#define DHTPIN 2
//#define DHTTYPE DHT11
DHT dht(DHTPIN, DHT11);

using namespace std;

WiFiClient net;
PubSubClient client(net);

unsigned long pollTimestamp = 0;
unsigned long pollDelay = 30000;

void publish(const String &topic, const String &payload)
{
    unsigned char topicChars[100];
    unsigned char payloadChars[100];
    topic.getBytes(topicChars, 100);
    payload.getBytes(payloadChars, 100);
    client.publish((char *)topicChars, (char *)payloadChars); //,false,1);
}

void blink(int count)
{
    for (int i = 0; i < count; i++)
    {
        digitalWrite(LED_PIN, LED_ON);
        delay(100);
        digitalWrite(LED_PIN, LED_OFF);
        delay(500);
    }
    delay(1000);
}

void sensorState(bool state)
{
    if (state == ON)
    {
        digitalWrite(SLEEP_PIN, HIGH);
    }
    else
    {
        digitalWrite(SLEEP_PIN, LOW);
    }
}

void readTemperature(){
    dht.begin();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    publish("/air2/humidity", String(h));
    publish("/air2/temperature", String(t));

    // set it back to output
    pinMode(2, OUTPUT);   
}

void messageReceived(const String topic, const String payload)
{
    if (topic == String("/air2/poll"))
    {
        publish("/air2/ack", "poll");
        pollTimestamp = millis();
        sensorState(ON);
        readTemperature();
        blink(1);
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
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(200);
        blink(2);
    }

    client.setServer("192.168.2.10", 1883);
    while (!client.connect("air2"))
    {
        delay(200);
        blink(3);
    }
    client.subscribe("/air/#");
    client.subscribe("/air2/#");
    client.setCallback(pubSubCallback);
}

void setup()
{
    pinMode(2, OUTPUT);
    sensorState(OFF);
    Serial.begin(9600);
    connect();
    publish("/air2/IP", WiFi.localIP().toString());
    publish("/air2/version", "2");
    pinMode(0, OUTPUT);
    sensorState(ON);
    blink(5);
    sensorState(OFF);
    
    dht.begin();
}

bool gotIt = false;

String readShort()
{
    int value = Serial.read() << 8;
    value += Serial.read();
    return String(value);
}

String readSet()
{
    return (readShort() + " " + readShort() + " " + readShort());
}

void loop()
{

    client.loop();
    if (!client.connected())
    {
        connect();
    }
    delay(50);

    if (pollTimestamp != 0 && millis() - pollTimestamp > pollDelay)
    {
        if (!gotIt && Serial.available() > 1)
        {
            // read the incoming byte:
            int incomingByte = Serial.read();
            if (incomingByte == 0x42)
            {
                int nextByte = Serial.read();
                if (nextByte == 0x4d)
                {
                    gotIt = true;
                    //publish("/air2/debug", "header");
                }
            }
        }
        if (gotIt && Serial.available() >= 12)
        {
            // got it!
            // String referenceValues = readSet();
            const String frameLength = readShort();
            // publish("/air2/debug/reference", referenceValues);
            publish("/air2/pm01", readShort());
            publish("/air2/pm25", readShort());
            publish("/air2/pm10", readShort());
            publish("/air2/cu", readShort());
            /* 
            String atmosphericValues = readSet();
            publish("/air/value/atmosphere", atmosphericValues);
            */
            sensorState(OFF);
            pollTimestamp = 0;
            gotIt = false;
        }
    }
    else
    {
        int trash = 0;
        while (Serial.available() > 0)
        {
            trash = Serial.read();
        }
    }
}

/*
    // simple blinking on D1
  */
/*
    while (true) {
        delay(10);

        utils.loop();

        for (int i = 0; i <= 8; i++) {
            const int delta = abs(level[i] - target[i]);
            // this catches the last step before reaching the target
            if (level[i] == -1 || delta <= speed[i]) {
                level[i] = target[i];
            } else {
                if (level[i] > target[i]) {
                    level[i] -= speed[i];
                } else {
                    level[i] += speed[i];
                }
            }
            analogWrite(pins[i], level[i]);
        }
    }
    */

int main()
{
}
