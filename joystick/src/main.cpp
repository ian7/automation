//#include <MQTTClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <wifi-password.h>

int oldSpeed=0;
int oldDirection=0;
bool oldHorn=false;

WiFiClient net;
PubSubClient client(net);
//MQTTClient client;

void connect()
{
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }
  while (!client.connect("joystick")) {
    delay(200);
  }
}

void messageReceived(String &topic, String &payload)
{

}



void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  digitalWrite(34, HIGH);
  digitalWrite(33, HIGH);

  pinMode(34,INPUT_PULLUP);
  pinMode(33,INPUT_PULLUP);
  pinMode(27,INPUT_PULLUP);
  digitalWrite(27, HIGH);
  
  client.setServer("10.10.4.1",1883);
  //client.begin("10.10.4.1", net);
  //client.onMessage(messageReceived);
//  client.setCallback(pubSubCallback);
  connect();
}


void loop() {
  while (true)
  {
    delay(30);

    client.loop();
    if (!client.connected())
    {
      connect();
    }
    char msg[20];
    int x = analogRead(34);
/*    int speed = (x-2048-580)*1.3;
    if( speed > 2040 ) {
      speed = 2040;
    }
    if( speed < -2040 ){
      speed = -2040;
    }

    */
    int speed = map( x+500,0,4096,-1000,1000)-200;

    if( abs( speed ) < 170 ){
      speed = 0;
    }

    if( abs( oldSpeed - speed ) > 40 ){
      oldSpeed = speed;
      snprintf (msg, 20, "%d", speed);
      client.publish("/car/drive",msg);//,false,1);
    }

    int y = analogRead(33);
    /*int direction = (y/22)-20;
    if( direction > 180 ){
      direction = 180;
    }
    if( direction < 0 ){
      direction = 0;
    }*/
    int direction = map( y, 0, 4096,50,150);

    if( abs( oldDirection-direction) > 3 ){
      snprintf (msg, 20, "%d", direction);
      client.publish("/car/steering",msg);//,false,1);
      oldDirection = direction;
    }

    bool horn = (digitalRead(27)==LOW);
    if( horn != oldHorn ){
      if( horn == true) {
      client.publish("/car/horn","on");
      }
      else { 
      client.publish("/car/horn","off");
      }
      oldHorn = horn;
    }
  }
}