
#define _GLIBCXX_USE_C99 1

#include <Arduino.h>

#define LED_PIN 2
#define LED_ON 0
#define LED_OFF 1

#define ON true
#define OFF false

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include "wifi-password.h"

#include <PubSubClient.h>

#include <string>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>

#include "encoder.h"

using namespace std;

WiFiClient net;
PubSubClient client(net);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver leds = Adafruit_PWMServoDriver(0x41);

Encoder encoder(12, 13,0,31);

int dark[] = {0,0,0,0,0};
int dim[] = {3,3,3,6,6};
int full[] = {80,20,25,70,60};

unsigned long pollTimestamp = 0;
unsigned long pollDelay = 30000;
int pm = 10;

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

void off()
{
    for (uint8_t pwmnum = 0; pwmnum < 4; pwmnum++)
    {
        pwm.setPWM(pwmnum, 0, 0);
    }
}

void set(int i, int val)
{
    pwm.setPWM(i, 0, val);
}

void setInv(int i, int val)
{
    pwm.setPWM(i, 4090-val, 4090);
}


void setLed(int i, int val)
{
    leds.setPWM(i, 0, val);
}

void messageReceived(const String topic, const String payload)
{
    if (topic == String("/purifier/fans"))
    {
        int power = payload.toInt();
        publish("/air/ack", "fans: " + payload);
        set(0, power);
        set(1, power);
        set(2, power);
        set(3, power);
    }
    if (topic == String("/purifier/fansi"))
    {
        int power = payload.toInt();
        publish("/air/ack", "fans: " + payload);
        set(0, power);
        set(1, power);
        set(2, power);
        setInv(3, power);
    }
    if (topic == String("/purifier/fan1"))
    {
        int power = payload.toInt();
        publish("/air/ack", "fan1: " + payload);
        set(0, power);
    }
    if (topic == String("/purifier/fan2"))
    {
        int power = payload.toInt();
        publish("/air/ack", "fan2: " + payload);
        set(1, power);
    }
    if (topic == String("/purifier/fan3"))
    {
        int power = payload.toInt();
        publish("/air/ack", "fan3: " + payload);
        set(2, power);
    }
    if (topic == String("/purifier/fan4"))
    {
        int power = payload.toInt();
        publish("/air/ack", "fan3: " + payload);
        set(3, power);
    }
    if (topic == String("/purifier/fan4"))
    {
        int power = payload.toInt();
        publish("/air/ack", "fan3: " + payload);
        set(3, power);
    }
    if (topic == String("/purifier/led1"))
    {
        int power = payload.toInt();
        publish("/air/ack", "led1: " + payload);
        setLed(0, power);
    }
    if (topic == String("/purifier/led2"))
    {
        int power = payload.toInt();
        publish("/air/ack", "led2: " + payload);
        setLed(1, power);
    }
    if (topic == String("/purifier/led3"))
    {
        int power = payload.toInt();
        publish("/air/ack", "led3: " + payload);
        setLed(2, power);
    }
    if (topic == String("/purifier/led4"))
    {
        int power = payload.toInt();
        publish("/air/ack", "led4: " + payload);
        setLed(3, power);
    }
    if (topic == String("/purifier/led5"))
    {
        int power = payload.toInt();
        publish("/air/ack", "led5: " + payload);
        setLed(4, power);
    }

    if (topic == String("/purifier/frequency"))
    {
        int frequency = payload.toInt();
        publish("/air/ack", "freq: " + payload);
        pwm.setPWMFreq(frequency);
    }
    if (topic == String("/air/pm01"))
    {
        pm = payload.toInt();
        publish("/purifier/ack", "pm: " + payload);
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

    client.setServer("10.10.4.1", 1883);
    while (!client.connect("purifier"))
    {
        delay(200);
        blink(3);
    }
    client.subscribe("/purifier/#");
    client.subscribe("/air/#");
    client.setCallback(pubSubCallback);
}

int timestamp;

void setup()
{
    pinMode(2, OUTPUT);
    pinMode(14, OUTPUT);
    Serial.begin(9600);
    connect();

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

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });

    ArduinoOTA.setHostname("purifier");
    ArduinoOTA.begin();

    publish("/purifier/IP", WiFi.localIP().toString());
    publish("/purifier/version", "10");
    blink(2);

    pwm.begin();
    pwm.setPWMFreq(15000); // This is the maximum PWM frequency
    leds.begin();
    leds.setPWMFreq(100);
    Wire.setClock(100000);
    timestamp = millis();
    pinMode(15, INPUT);
}

int lastValue = -1;

void setFans( int value ){
    set( 0, value );
    set( 1, value );
    set( 2, value );
    set( 3, value );
}

void ledsOff(){
    for( int i=0;i<=5;i++){
        setLed(i, 0);
    }
}

void setLeds( int value, int * brightness ){
    ledsOff();
    if( value > 0 ){
        setLed(0,brightness[0]);
    }
    if( value > 1300 ){
        setLed(1,brightness[1]);
    }
    if( value > 2100 ){
        setLed(2,brightness[2]);
    }
    if( value > 3000){
        setLed(3,brightness[3]);
    }
    if( value > 4000){
        setLed(4,brightness[4]);
    }
}

int ledStamp = -1;

void loop()
{
    client.loop();
    if (!client.connected())
    {
        connect();
    }
    ArduinoOTA.handle();

    int now = millis();

    if( encoder.check() ) {
        int power = encoder.getPosition()*100+995;
        if( power < 1000 ){
            power = 0;
        }
        publish("/purifier/power", String(power));
        set(0, power);
        set(1, power);
        set(2, power);
        //if( power > 3000 ){
            set(3, power);
        //}
        //else{
        //    set(3,0);
        //}
        setLeds(power,full);
        ledStamp = now;
    }


    if (timestamp + 10 < now)
    {
        int value = digitalRead(15);
        if (lastValue != value)
        {
            publish("/purifier/button", String(value));
            lastValue = value;
            if( value == 0 ){
                set(0, 0);
                set(1, 0);
                set(2, 0);
                set(3, 0);
                setLeds(0,full);
                ledStamp = now;
            }
        }
        timestamp = now;
    }
    if( ledStamp + 3000 < now ){
        setLeds( pm*30, dim);
        ledStamp = now;
    }
}
