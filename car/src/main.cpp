#include <Servo.h>
//#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include <wifi-password.h>
#include <PubSubClient.h>

WiFiClient net;
PubSubClient client(net);
//MQTTClient client;
Servo servo;

void publish( const String &topic, const String &payload){
    unsigned char topicChars[100];
    unsigned char payloadChars[100];
    topic.getBytes(topicChars,100);
    payload.getBytes(payloadChars,100);
    client.publish((char *) topicChars, (char *) payloadChars);//,false,1);
}

void messageReceived(const String topic, const String payload)
{

  if (topic == String("/car/steering"))
  {
    int steering = payload.toInt();
    servo.write(steering);
    //publish("/car/ack", "steering: " + String(steering));
  }

  if (topic == String("/car/horn"))
  {
    if( payload == String("on") ){
      digitalWrite(10,HIGH);
    }

    if( payload == String("off") ){
      digitalWrite(10,LOW);
    }
//    publish("/car/ack", "horn: " + payload);
  }

  if (topic == String("/car/drive"))
  {
    int speed = payload.toInt();

    //publish("/car/ack", "drive: " + String(speed));

    if (speed > 0)
    {
      digitalWrite(0, LOW);
      analogWrite(5, speed);
    }
    else
    {
      digitalWrite(0, HIGH);
      analogWrite(5, -speed);
    }
  }
}

void connect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
  }
  while (!client.connect("car"))
  {
    delay(200);
  }
  client.subscribe("/car/drive", 1);
  client.subscribe("/car/steering", 1);
  client.subscribe("/car/horn", 1);
}

void pubSubCallback(char* topic, byte* payload, unsigned int length){
    String payloadString = String((char*)payload).substring(0,length);
    String topicString = String(topic);
    messageReceived(topicString, payloadString);
}

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  pinMode(0, OUTPUT);
  pinMode(10, OUTPUT);

  //client.begin("10.10.4.1", net);
  //client.onMessage(messageReceived);
  client.setServer("10.10.4.1",1883);
  client.setCallback(pubSubCallback);
  connect();
  servo.attach(2);
}

void loop()
{
  while (true)
  {
    delay(50);

    client.loop();
    if (!client.connected())
    {
      connect();
    }
  }

  /*
  delay(1000);
  analogWrite(5, 1020);
  delay(1000);
  analogWrite(5, 700);
  delay(1000);
  analogWrite(5, 0);
  digitalWrite(0, 0);
  delay(1000);
  servo.write(0);
  delay(1000);
  delay(1000);
  analogWrite(5, 700);
  delay(1000);
  analogWrite(5, 1020);
  delay(1000);
  analogWrite(5, 700);
  delay(1000);
  analogWrite(5, 0);
  digitalWrite(0, 1);
  */
}