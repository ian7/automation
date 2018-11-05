#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <wifi-password.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>


#include "DHT.h"
#define TEMP1 4
#define TEMP2 0
#define TEMP3 2
#define DHTTYPE DHT11 
DHT dht1(TEMP1, DHT11);
DHT dht2(TEMP2, DHT11);
DHT dht3(TEMP3, DHT11);


const char projectName[] = "bathroom";

#include <PubSubUtils.h>

#define ENA 16
#define IN1 14
#define IN2 12
#define IN3 13
#define IN4 15
#define ENB 5


int oldA = 0;
int oldB = 0;
int speedA = 0;
int speedB = 0;
int position = 0;
const int version = 7;


void setDirectionA( bool forward ){
  if( forward ){
    digitalWrite(IN1,0);
    digitalWrite(IN2,1);
  }
  else{
    digitalWrite(IN1,1);
    digitalWrite(IN2,0);
  }
}

void setDirectionB( bool forward ){
  if( forward ){
    digitalWrite(IN3,0);
    digitalWrite(IN4,1);
  }
  else{
    digitalWrite(IN3,1);
    digitalWrite(IN4,0);
  }
}

void setSpeedA(int speed){
  setDirectionA(speed>0);
  analogWrite(ENA,abs(speed));
}

void setSpeedB(int speed){
  setDirectionB(speed>0);
  analogWrite(ENB,abs(speed));
}


void readDHT( DHT &sensor, String sensorID ){
  float humidity = sensor.readHumidity();
  float temperature = sensor.readTemperature();
  String topic = String("/")+projectName+"/"+sensorID+"/";
  publish(topic+"temp", String(temperature));
  publish(topic+"humidity", String(humidity));
}

void messageReceived(const String topic, const String payload)
{
  if (topic == String("/")+projectName+"/speedA")
  {
    int speed = payload.toInt();
    ack("speedA: " + String(speed));
    setSpeedA(speed);
  }

  if (topic == String("/")+projectName+"/speedB")
  {
    int speed = payload.toInt();
    ack("speedB: " + String(speed));
    setSpeedB(speed);
  }

  if (topic == String("/")+projectName+"/poll")
  {
    readDHT(dht1,"1");
    readDHT(dht2,"2");
    readDHT(dht3,"3");
  }

}

void connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    blink( 2 );
    //ESP.restart();
  }
  client.setServer("10.10.4.1",1883);

  while (!client.connect(projectName))
  {
    delay(200);
    blink( 3 );
  }
  client.subscribe((String("/")+projectName+"/+").c_str(), 1);

  publish(String("/")+projectName+"/IP",WiFi.localIP().toString());
  publish(String("/")+projectName+"/version",String(version));
}

void pubSubCallback(char* topic, byte* payload, unsigned int length){
    String payloadString = String((char*)payload).substring(0,length);
    String topicString = String(topic);
    messageReceived(topicString, payloadString);
}

void setPinModes(){
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void setup()
{
  setPinModes();
  analogWriteFreq(12); 
  
  client.setCallback(pubSubCallback);
  connect();
  
  setupOTA();
  dht1.begin();
  dht2.begin();
  dht3.begin();
}

void loop()
{  
    ArduinoOTA.handle();

    client.loop();
    if (!client.connected())
    {
      connect();
    }

    delay(50);

}