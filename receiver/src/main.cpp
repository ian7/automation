
#define _GLIBCXX_USE_C99 1

#ifdef ESP8266
#include <Arduino.h>
#include <RCSwitch.h>
#endif

#define LED_PIN 2

#define LED_ON 0
#define LED_OFF 1

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "wifi-password.h"


#include <string>

using namespace std;

RCSwitch mySwitch = RCSwitch();
WiFiClient net;
PubSubClient client(net);

void publish(const String &topic, const String &payload)
{
  unsigned char topicChars[100];
  unsigned char payloadChars[100];
  topic.getBytes(topicChars, 100);
  payload.getBytes(payloadChars, 100);
  client.publish((char *)topicChars, (char *)payloadChars); //,false,1);
}

void blink( int count){
    for( int i=0; i<count; i++){
            digitalWrite(LED_PIN,LED_ON);
            delay(100);
            digitalWrite(LED_PIN,LED_OFF);
            delay(500);
    }
    delay(1000);
}

void connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    blink( 2 );
  }
 
  client.setServer("10.10.4.1", 1883);
  while (!client.connect("receiver"))
  {
    delay(200);
    blink( 3 );
  }
}

void messageReceived(const string topic, const string payload) {
}

void setup() {
    pinMode(2,OUTPUT);
    mySwitch.enableReceive(3);
    connect();
    publish("/receiver/IP", WiFi.localIP().toString());
    publish("/receiver/version", "1");
    blink(5);
}



void loop() {

/*    while(true){
    }
    */
    client.loop();
    if (!client.connected())
    {
      connect();
    }
    delay(50);
        if (mySwitch.available()) {
            publish("/receiver/value",String(mySwitch.getReceivedValue()));
            publish("/receiver/protocolValue",String(mySwitch.getReceivedProtocol()) + String(mySwitch.getReceivedValue()));
            mySwitch.resetAvailable();
            digitalWrite(LED_PIN,LED_ON);
            delay(100);
            digitalWrite(LED_PIN,LED_OFF);
            delay(100);
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


int main() {
}
