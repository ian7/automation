#include <MQTTClient.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <wifi-password.h>
#include <HTTPClient.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

WiFiClient net;
MQTTClient client;

const int version = 4;
const char projectName[] = "bedroom-light";

bool lastTopLight = false;
unsigned long lastHeartBeat = 0;
const unsigned long heartBeatInterval = 10000; // 10 seconds
const int reconnectCounterThreshold = 100; // retires happen every 200ms, so this gives 20s

int currentw = 0;
int cW = 0, cR = 0, cG = 0, cB = 0;
int w = 0, r = 0, g = 0, b = 0;
int cH = 0, cS = 0, cV = 0;
int h = 0, s = 0, v = 0;

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0 0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT 13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 125

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN 23

int fadeAmount = 5; // how many points to fade the LED by

void hard_restart() {
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}

void hsi2rgbw(float H, float S, float I, int *rgbw)
{
  int r, g, b, w;
  float cos_h, cos_1047_h;
  H = fmod(H, 360);                // cycle H around to 0-360 degrees
  H = 3.14159 * H / (float)180;    // Convert to radians.
  S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
  I = I > 0 ? (I < 1 ? I : 1) : 0;

  if (H < 2.09439)
  {
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667 - H);
    r = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    g = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    b = 0;
    w = 255 * (1 - S) * I;
  }
  else if (H < 4.188787)
  {
    H = H - 2.09439;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667 - H);
    g = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    b = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    r = 0;
    w = 255 * (1 - S) * I;
  }
  else
  {
    H = H - 4.188787;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667 - H);
    b = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    r = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    g = 0;
    w = 255 * (1 - S) * I;
  }

  rgbw[0] = r;
  rgbw[1] = g;
  rgbw[2] = b;
  rgbw[3] = w;
}

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * std::min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void connect()
{

  int reconnectCounter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    reconnectCounter++;
    if(reconnectCounter > reconnectCounterThreshold){
      hard_restart();
    }
  }

  reconnectCounter=0;
  while (!client.connect(projectName))
  {
    delay(200);
    reconnectCounter++;
    if(reconnectCounter > reconnectCounterThreshold){
      hard_restart();
    }
  }


  client.subscribe("/light/bedroom/+");
  client.subscribe("/heartbeat");
}

void messageReceived(String &topic, String &payload)
{
  if (topic == String("/heartbeat")){
    lastHeartBeat = millis();
  }

  if (topic == String("/light/bedroom/restart")){
    esp_restart();
  }

  if (topic == String("/light/bedroom/w"))
  {
    w = payload.toInt();
  }

  if (topic == String("/light/bedroom/r"))
  {
    r = payload.toInt();
  }
  if (topic == String("/light/bedroom/g"))
  {
    g = payload.toInt();
  }
  if (topic == String("/light/bedroom/b"))
  {
    b = payload.toInt();
  }
  if (topic == String("/light/bedroom/hue"))
  {
    h = payload.toInt();
  }
  if (topic == String("/light/bedroom/saturation"))
  {
    s = payload.toInt();
  }
  if (topic == String("/light/bedroom/brightness"))
  {
    v = payload.toInt();
  }
  if (topic == String("/light/bedroom/color"))
  {
    char colorString[100];
    //unsigned int
    payload.toCharArray(colorString, 100);
    sscanf(colorString, "#%02x%02x%02x", &r, &g, &b);
    //h = payload.toInt();
  }
}

void setup()
{
  // Setup timer and attach timer to a led pin
  ledcSetup(0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(23, 0);
  ledcSetup(1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(21, 1);
  ledcSetup(2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  // this was ment to be 19, but I burned out pin on the mini32 and it is bridged over to 33
  ledcAttachPin(33, 2);
  ledcSetup(3, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(18, 3);

  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  client.begin("10.10.4.1", net);
  client.onMessage(messageReceived);
  connect();
  client.publish("/bedroom-light/IP", WiFi.localIP().toString());
  client.publish("/bedroom-light/RSSI", String(WiFi.RSSI()));
  client.publish("/bedroom-light/version", String(version));

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
  lastHeartBeat = millis();
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

    // set the w on LEDC channel 0

    if (w != cW)
    {
      if (cW > w)
      {
        cW--;
      }
      else
      {
        cW++;
      }
      ledcAnalogWrite(1, cW);
      delay(1);
    }

    if (r != cR)
    {
      if (cR > r)
      {
        cR--;
      }
      else
      {
        cR++;
      }
      ledcAnalogWrite(2, cR);
      delay(1);
    }

    if (g != cG)
    {
      if (cG > g)
      {
        cG--;
      }
      else
      {
        cG++;
      }
      ledcAnalogWrite(0, cG);
      delay(1);
    }

    if (b != cB)
    {
      if (cB > b)
      {
        cB--;
      }
      else
      {
        cB++;
      }
      ledcAnalogWrite(3, cB);
      delay(1);
    }

    int rgbw[4];

    if (v != cV)
    {
      if (cV > v)
        cV--;
      else
        cV++;
      hsi2rgbw(((float)cH) / 255 * 360, ((float)cS) / 255, ((float)cV) / 255, rgbw);
      ledcAnalogWrite(0, rgbw[3]);
      ledcAnalogWrite(1, rgbw[0]);
      ledcAnalogWrite(2, rgbw[1]);
      ledcAnalogWrite(3, rgbw[2]);
    }

    if (h != cH)
    {
      if (cH > h)
        cH--;
      else
        cH++;
      hsi2rgbw(((float)cH) / 255 * 360, ((float)cS) / 255, ((float)cV) / 255, rgbw);
      ledcAnalogWrite(0, rgbw[3]);
      ledcAnalogWrite(1, rgbw[0]);
      ledcAnalogWrite(2, rgbw[1]);
      ledcAnalogWrite(3, rgbw[2]);
    }

    if (s != cS)
    {
      if (cS > s)
        cS--;
      else
        cS++;
      hsi2rgbw(((float)cH) / 255 * 360, ((float)cS) / 255, ((float)cV) / 255, rgbw);
      ledcAnalogWrite(0, rgbw[3]);
      ledcAnalogWrite(1, rgbw[0]);
      ledcAnalogWrite(2, rgbw[1]);
      ledcAnalogWrite(3, rgbw[2]);
    }

    // wait for 30 milliseconds to see the dimming effect
    delay(5);

    bool currentTopLight = digitalRead(27);

    if( currentTopLight != lastTopLight ){
      // reading one is no singal, reading zero is signal
      client.publish("/light/bedroom/topLight",String(!currentTopLight));
      lastTopLight=currentTopLight;
    }
    ArduinoOTA.handle();

    unsigned long timeNow = millis();
    if( lastHeartBeat != 0 && timeNow - lastHeartBeat > heartBeatInterval ){
      client.publish("/light/bedroom/watchdog","triggered");
      lastHeartBeat = 0;
      hard_restart();
    }
  }
}
