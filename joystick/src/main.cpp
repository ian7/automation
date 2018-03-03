#include <MQTTClient.h>
#include <WiFi.h>
//#include <PubSubClient.h>
#include <wifi-password.h>

WiFiClient net;
//PubSubClient client(net);
MQTTClient client;

void connect()
{
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  while (!client.connect("joystick")) {
    delay(1000);
  }
}

void messageReceived(String &topic, String &payload)
{

}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  //client.setServer("10.10.1.3",1883);
  client.begin("10.10.4.1", net);
  client.onMessage(messageReceived);
  connect();
}


void loop() {
  while (true)
  {
    delay(100);

    client.loop();
    if (!client.connected())
    {
      connect();
    }
    char msg[20];
    int x = analogRead(32);
    int speed = (x-2048-580)*1.3;
    if( speed > 2040 ) {
      speed = 2040;
    }
    if( speed < -2040 ){
      speed = -2040;
    }
    snprintf (msg, 20, "%d", speed);
    client.publish("/car/drive",msg);//,false,1);
    int y = analogRead(33);
    int direction = (y/22)-20;
    if( direction > 180 ){
      direction = 180;
    }
    if( direction < 0 ){
      direction = 0;
    }
    snprintf (msg, 20, "%d", direction);
    client.publish("/car/steering",msg);//,false,1);
  }
}