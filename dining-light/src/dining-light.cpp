#define FASTLED_ALLOW_INTERRUPTS 0
//#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
//#define ESP8266

#include <Arduino.h>
#include <MQTTClient.h>
#include <WiFi.h>
#include <wifi-password.h>
#include "FastLED.h"

WiFiClient net;
MQTTClient client;

#define NUM_LEDS 150
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0 0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT 13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 125
//000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN 5

int brightness = 0; // how bright the LED is
int fadeAmount = 5; // how many points to fade the LED by
int oldBrightness = 0;

int xmassColorStart = 0x60;
int xmassColorStop = 0x85;
int xmassColor = xmassColorStart;
int xmassDelay = 1000;
int xmassBrigthness = 255;
int xmassTimestamp = millis();
int xmassDirection = 1;
enum { up=1, down=-1} direction;

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
        // calculate duty, 8191 from 2 ^ 13 - 1
        uint32_t duty = (8191 / valueMax) * std::min(value, valueMax);

        // write duty to LEDC
        //        ledcWrite(channel, duty);
        //analogWrite(channel, duty);
}

void connect()
{
        while (WiFi.status() != WL_CONNECTED)
        {
                delay(1000);
        }

        while (!client.connect("diningRoomLight"))
        {
                delay(1000);
        }
        client.subscribe("/light/diningRoom/brightness");
        client.subscribe("/light/xmass");
        client.subscribe("/light/xmass/brightness");
        client.subscribe("/light/xmass/start");
        client.subscribe("/light/xmass/stop");
        client.subscribe("/light/xmass/delay");
}

void messageReceived(String &topic, String &payload)
{

        if (topic == String("/light/diningRoom/brightness")) {
                brightness = payload.toInt();
                client.publish("/light/ack", "diningRoom b: " + String(brightness));
        }
        if (topic == String("/light/xmass/brightness")) {
                xmassBrigthness = payload.toInt();
                client.publish("/light/ack", "diningRoom xmass b: " + String(brightness));
        }
        if (topic == String("/light/xmass/start")) {
                xmassColorStart = payload.toInt();
                client.publish("/light/ack", "diningRoom xmass start: " + String(brightness));
        }
        if (topic == String("/light/xmass/stop")) {
                xmassColorStop = payload.toInt();
                client.publish("/light/ack", "diningRoom xmass stop: " + String(brightness));
        }
        if (topic == String("/light/xmass/delay")) {
                xmassDelay = payload.toInt();
                client.publish("/light/ack", "diningRoom xmass delay: " + String(brightness));
        }

        if (topic == String("/light/xmass"))
        {
                if (payload == String("off"))
                {
                        for (int i = 0; i < 150; i++)
                        {
                                leds[i] = CRGB(0, 0, 0);
                        }
                        FastLED.show();
                        client.publish("/light/ack", "xmass off");
                }
                else
                {
                        String colorString = payload.substring(0, 7);
                        String ledString = payload.substring(8);

                        char ledStringBuffer[10];
                        ledString.toCharArray(ledStringBuffer, 10);
                        char colorStringBuffer[10];
                        colorString.toCharArray(colorStringBuffer, 10);

                        // Get rid of '#' and convert it to integer
                        int colorNumber = (int)strtol(&colorStringBuffer[1], NULL, 16);
                        int i = ledString.toInt();

                        if (colorStringBuffer[0] == '#')
                        {
                                // Split them up into r, g, b values
                                int r = colorNumber >> 16;
                                int g = colorNumber >> 8 & 0xFF;
                                int b = colorNumber & 0xFF;

                                leds[i] = CRGB(b, r, g);

                                client.publish("/light/ack", "diningRoom l: " + String(i) + " r: " + String(r) + " g: " + String(g) + " b: " + String(b) + " " + String(ledStringBuffer));
                        }

                        if (colorStringBuffer[0] == 'c'){
                                int v = colorNumber >> 16;
                                int s = colorNumber >> 8 & 0xFF;
                                int h = colorNumber & 0xFF;

                                int i = ledString.toInt();
                                leds[i] = CHSV(h, s, v);

                                client.publish("/light/ack", "diningRoom l: " + String(i) + " h: " + String(h) + " s: " + String(s) + " v: " + String(v) + " " + String(ledStringBuffer));

                        }
                        FastLED.show();
                }
        }
}

void setup()
{
        // Setup timer and attach timer to a led pin
        ledcSetup(0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
        ledcAttachPin(23, 0);
        //analogWriteFreq(LEDC_BASE_FREQ);
        //analogWriteRange(1023);

        WiFi.begin(ssid, pass);

        client.begin("10.10.4.1", net);
        client.onMessage(messageReceived);
        connect();

        LEDS.addLeds<WS2812, 22, RGB>(leds, NUM_LEDS);
        LEDS.setBrightness(250);
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

                // set the brightness on LEDC channel 0

                if (brightness != oldBrightness)
                {
                        if (brightness > oldBrightness)
                        {
                                oldBrightness++;
                        }
                        else
                        {
                                oldBrightness--;
                        }

                        int hardwareBrightness = oldBrightness * 32;

                        client.publish("/light/ack", "diningRoom ob: " + String(hardwareBrightness));
                        //analogWrite(12, oldBrightness * 4);
                        ledcWrite(0, hardwareBrightness);
                        // wait for 30 milliseconds to see the dimming effect   
                        delay(3);
                }
                else {
                        delay(50);                
                }

                // if the change delay has elapsed - let's pick next color
                if( millis() - xmassTimestamp > xmassDelay ){
                        xmassTimestamp = millis();

                        // going up
                        if( xmassDirection == up ){
                                if( xmassColor < xmassColorStop ){
                                        xmassColor++;
                                }
                                else {
                                        xmassDirection = down;
                                }
                        }
                        else{
                        // going down
                                if( xmassColor > xmassColorStart ){
                                        xmassColor--;
                                }
                                else {
                                        xmassDirection = up;
                                }
                        }
                        for( int i=0; i<50;i++){
                                leds[i] = CHSV(xmassColor, 255, xmassBrigthness);
                        }
                        FastLED.show();
                }
        }
}
