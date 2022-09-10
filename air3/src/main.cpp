
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
#include <PMserial.h>
#include <ArduinoOTA.h>



#include <string>

#include <DHT.h>

#define DHTPIN 2
//#define DHTTYPE DHT11
//DHT dht(DHTPIN, DHT11);

SerialPM pms(PMSx003, Serial);  // PMSx003, UART

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
 //   dht.begin();
//    float h = dht.readHumidity();
 //   float t = dht.readTemperature();
//    publish("/air2/humidity", String(h));
//    publish("/air2/temperature", String(t));

    // set it back to output
//    pinMode(2, OUTPUT);   
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

void pubSubCallback(char *topic, unsigned char *payload, unsigned int length)
{
    String payloadString = String((char *)payload).substring(0, length);
    String topicString = String(topic);
    messageReceived(topicString, payloadString);
}

void connect()
{
    WiFi.mode(WIFI_STA);
    WiFi.hostname("IOT-air3");
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(200);
        blink(2);
    }

    client.setServer("192.168.0.5", 1883);
    while (!client.connect("air3"))
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
    
    pms.init();                   // config serial port
    
    connect();
    publish("/air2/IP", WiFi.localIP().toString());
    publish("/air2/version", "4");
    pinMode(0, OUTPUT);
    sensorState(ON);
    blink(5);
    sensorState(OFF);


    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        { // U_SPIFFS
            type = "filesystem";
        }
    });

    ArduinoOTA.setHostname("air3");
    ArduinoOTA.begin();

    
 //   dht.begin();
}

bool gotIt = false;



void loop()
{

    client.loop();
    if (!client.connected())
    {
        connect();
    }
    delay(50);
    ArduinoOTA.handle();
    
    if (pollTimestamp != 0 && millis() - pollTimestamp > pollDelay)
    {

          pms.read();                   // read the PM sensor

            publish("/air2/pm01", String(pms.pm01));
            publish("/air2/pm25", String(pms.pm25));
            publish("/air2/pm10", String(pms.pm10));

            sensorState(OFF);
            pollTimestamp = 0;
            gotIt = false;
    }
}