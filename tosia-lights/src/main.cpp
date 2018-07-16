#include <Arduino.h>

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER

#include <FastLED.h>
#include <MQTTClient.h>
#include <WiFi.h>
#include "wifi-password.h"

WiFiClient net;
MQTTClient client;

#include <IRremote.h>

uint16_t RECV_PIN = 23;
IRrecv irrecv(RECV_PIN);
decode_results results;

// How many leds in your strip?
#define NUM_LEDS 50

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 22
#define CLOCK_PIN 2

int red = 10;
int green = 10;
int blue = 10;

int hue = 10;
int saturation = 10;
int brightness = 10;

int oldHue = hue;
int oldSaturation = saturation;
int oldBrightness = brightness;

int oldRed = red;
int oldGreen = green;
int oldBlue = blue;

// this is defined in wifi-password.h
//const char ssid[] = "xxx";
//const char pass[] = "xxx";

// Define the array of leds
CRGB leds[NUM_LEDS];

void messageReceived(String &topic, String &payload)
{

  if (topic == String("/light/tosia/hue"))
  {
    hue = payload.toInt();
    client.publish("/light/tosia/ack", "h: " + String(hue) + " s: " + String(saturation) + " b: " + String(brightness));
  }
  if (topic == String("/light/tosia/saturation"))
  {
    saturation = payload.toInt();
    client.publish("/light/tosia/ack", "h: " + String(hue) + " s: " + String(saturation) + " b: " + String(brightness));
  }
  if (topic == String("/light/tosia/brightness"))
  {
    brightness = payload.toInt();
    client.publish("/light/tosia/ack", "h: " + String(hue) + " s: " + String(saturation) + " b: " + String(brightness));
  }

  if (topic == String("/light/tosia/red"))
  {
    red = payload.toInt();
    client.publish("/light/tosia/ack", "red " + payload);
  }
  if (topic == String("/light/tosia/green"))
  {
    red = payload.toInt();
    client.publish("/light/tosia/ack", "green " + payload);
  }
  if (topic == String("/light/tosia/blue"))
  {
    red = payload.toInt();
    client.publish("/light/tosia/ack", "blue " + payload);
  }
  //show();
}

void connect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  while (!client.connect("tosia-light"))
  {
    delay(1000);
  }
  client.subscribe("/light/tosia/red");
  client.subscribe("/light/tosia/green");
  client.subscribe("/light/tosia/blue");
  client.subscribe("/light/tosia/hue");
  client.subscribe("/light/tosia/saturation");
  client.subscribe("/light/tosia/brightness");
}

void setup()
{
  LEDS.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS, 0);
  LEDS.setBrightness(250);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  client.begin("10.10.1.3", net);
  client.onMessage(messageReceived);
  connect();

  irrecv.enableIRIn();
  //myReceiver.enableIRIn(); // Start the receiver
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

    if (oldHue != hue || oldSaturation != saturation || oldBrightness != brightness)
    {
      /*if( hue > oldHue ){
        oldHue++;
      }
      
      if( hue < oldHue ){
        oldHue--;
      }

      if( saturation > oldSaturation) {
        oldSaturation++;
      }

      if( saturation < oldSaturation ){
        oldSaturation--;
      }

      if( brightness > oldBrightness ){
        oldBrightness++;
      }

      if( brightness < oldBrightness ){
        oldBrightness--;
      }
      */
      oldHue = hue;
      oldBrightness = brightness;
      oldSaturation = saturation;

      // First slide the led in one direction
      for (int i = 0; i < NUM_LEDS; i++)
      {
        // Set the i'th led to red
        CHSV newColor = CHSV(oldHue, oldSaturation, oldBrightness);
        hsv2rgb_rainbow(newColor, leds[i]);
        // Show the leds
      }
      FastLED.show();
    }

    if (irrecv.decode(&results))
    {
      if (results.decode_type == NEC)
      {
        uint64_t number = results.value;
        unsigned long long1 = (unsigned long)((number & 0xFFFF0000) >> 16);
        unsigned long long2 = (unsigned long)((number & 0x0000FFFF));

        String hex = String(long1, HEX) + String(long2, HEX); // six octets

        client.publish("/light/tosia/irReceiver", hex);
      }

      irrecv.resume(); // Receive the next value
    }

    delay(30);
  }
}