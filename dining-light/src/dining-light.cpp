#include <Arduino.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include <wifi-password.h>
#include "FastLED.h"    

WiFiClient net;
MQTTClient client;

#define NUM_LEDS 150
#define DATA_PIN 6

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

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
        // calculate duty, 8191 from 2 ^ 13 - 1
        uint32_t duty = (8191 / valueMax) * std::min(value, valueMax);

        // write duty to LEDC
//        ledcWrite(channel, duty);
        analogWrite(channel, duty);
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
}


void messageReceived(String &topic, String &payload)
{

        if (topic == String("/light/diningRoom/brightness"))
        {
                brightness = payload.toInt();
        }
        if (topic == String("/light/xmass"))
        {
                String colorString = payload.substring(0,7);
                String ledString = payload.substring( 8 );

                //string hexstring = "#FF3Fa0";
                char ledStringBuffer[10];
                ledString.toCharArray(ledStringBuffer,10);

                // Get rid of '#' and convert it to integer
                int colorNumber = (int) strtol( &ledStringBuffer[1], NULL, 16);

                // Split them up into r, g, b values
                int r = colorNumber >> 16;
                int g = colorNumber >> 8 & 0xFF;
                int b = colorNumber & 0xFF;

                //leds[i] = CHSV(hue++, 255, 255);
                int i = ledString.toInt();
                leds[i] = CRGB(r,g,b);

                //brightness = payload.toInt();
        }

        client.publish("/light/ack", "diningRoom b: " + String(brightness));
}

void setup()
{
        // Setup timer and attach timer to a led pin
        //ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
        //ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
        analogWriteFreq(LEDC_BASE_FREQ);
        analogWriteRange(1023);


        WiFi.begin(ssid, pass);

        client.begin("10.10.4.1", net);
        client.onMessage(messageReceived);
        connect();

	LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
	LEDS.setBrightness(255);
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

                if( brightness != oldBrightness ){
                        if( brightness > oldBrightness ){
                                oldBrightness--;
                        }
                        else{
                                oldBrightness++;
                        }

                ledcAnalogWrite(LEDC_CHANNEL_0, oldBrightness);
                }

                // wait for 30 milliseconds to see the dimming effect
                delay(30);
        }
}
