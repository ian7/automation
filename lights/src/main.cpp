#include <Arduino.h>

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include "FastLED.h"
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
//#include <WiFi.h>
#include "wifi-password.h"

WiFiClient net;
MQTTClient client;

// How many leds in your strip?
#define NUM_LEDS 150

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 5
#define CLOCK_PIN 2

int hue = 3;
int saturation = 3;
int brightness = 3;

int oldBrightness = 10;
int oldHue = 10;
int oldSaturation = 10;

// this is defined in wifi-password.h
//const char ssid[] = "xxx";
//const char pass[] = "xxx";

// Define the array of leds
CRGB leds[NUM_LEDS];

void messageReceived(String &topic, String &payload)
{

  if (topic == String("/light/hue"))
  {
    hue = payload.toInt();
  }
  if (topic == String("/light/saturation"))
  {
    saturation = payload.toInt();
  }
  if (topic == String("/light/brightness"))
  {
    brightness = payload.toInt();
  }
}

void connect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  while (!client.connect("bedroom-light"))
  {
    delay(1000);
  }
  client.subscribe("/light/hue");
  client.subscribe("/light/saturation");
  client.subscribe("/light/brightness");
}

void setup()
{
  LEDS.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS, 0);
  LEDS.setBrightness(255);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  client.begin("10.10.1.3", net);
  client.onMessage(messageReceived);
  connect();

  delay(1000);

  hue = 0;
  saturation = 0;
  brightness = 0;
  client.publish("/light/brightness", "0");
}

void loop()
{

  while (true)
  {
    delay(100);

    client.loop();
    if (!client.connected())
    {
      connect();
    }

    if (brightness != oldBrightness || hue != oldHue || saturation != oldSaturation)
    {

      oldBrightness = brightness;
      oldHue = hue;
      oldSaturation = saturation;

      client.publish("/light/ack", "h: " + String(hue) + " s: " + String(saturation) + " b: " + String(brightness));

      for (int i = 0; i < NUM_LEDS; i++)
      {
        CHSV newColor = CHSV(oldHue, oldSaturation, oldBrightness);
        hsv2rgb_rainbow(newColor, leds[i]);
      }
      FastLED.show();
      delay(5);
    }
  }
}