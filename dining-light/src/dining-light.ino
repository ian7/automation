#include <MQTTClient.h>
#include <WiFi.h>
#include <wifi-password.h>


WiFiClient net;
MQTTClient client;

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     125
//000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            23

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * std::min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void connect() {
        while (WiFi.status() != WL_CONNECTED) {
                delay(1000);
        }

        while (!client.connect("livingRoomLight")) {
                delay(1000);
        }
        client.subscribe("/light/diningRoom/brightness");
}

void setup() {
  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);

        WiFi.begin(ssid, pass);

        // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
        // You need to set the IP address directly.
        client.begin("10.10.4.1", net);
        client.onMessage(messageReceived);
      connect(); 
}


void messageReceived(String &topic, String &payload) {

        if( topic == String("/light/diningRoom/brightness") ) {
                if( payload == String("+") ){
                  brightness++;
                }
                else if( payload == String("-") ){
                  brightness--;
                }
                else{
                  brightness = payload.toInt();
                }

        client.publish("/light/ack","diningRoom b: " + String(brightness));
        }
}

void loop() {


    while( true ){

        client.loop();

        if (!client.connected()) {
                connect();
        }


  // set the brightness on LEDC channel 0

  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);

  // wait for 30 milliseconds to see the dimming effect
  delay(30);
    }
}
