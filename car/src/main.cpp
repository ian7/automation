#include <Servo.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include <wifi-password.h>

WiFiClient net;
MQTTClient client;
Servo servo;

void messageReceived(String &topic, String &payload)
{

  if (topic == String("/car/steering"))
  {
    int steering = payload.toInt();
    servo.write(steering);

    client.publish("/car/ack", "steering: " + String(steering));
  }

  if (topic == String("/car/drive"))
  {
    int speed = payload.toInt();

    client.publish("/car/ack", "drive: " + String(speed));

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
    delay(1000);
  }
  while (!client.connect("car"))
  {
    delay(1000);
  }
  client.subscribe("/car/drive", 1);
  client.subscribe("/car/steering", 1);
}

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  pinMode(0, OUTPUT);

  client.begin("10.10.4.1", net);
  client.onMessage(messageReceived);
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