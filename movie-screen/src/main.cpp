#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <wifi-password.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#define PIN_A 16
#define PIN_B 4

WiFiClient net;
PubSubClient client(net);

int oldA = 0;
int oldB = 0;
int speed = 0;
int position = 0;

void publish( const String &topic, const String &payload){
    unsigned char topicChars[100];
    unsigned char payloadChars[100];
    topic.getBytes(topicChars,100);
    payload.getBytes(payloadChars,100);
    client.publish((char *) topicChars, (char *) payloadChars);//,false,1);
}

void messageReceived(const String topic, const String payload)
{
  if (topic == String("/movieScreen/speed"))
  {
    speed = payload.toInt();

    publish("/movieScreen/ack", "speed: " + String(speed));
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
    //ESP.restart();
  }
  while (!client.connect("movieScreen"))
  {
    delay(200);
  }
  client.subscribe("/movieScreen/+", 1);
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

  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(0, OUTPUT);
  
  client.setServer("10.10.4.1",1883);
  client.setCallback(pubSubCallback);
  connect();
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.setHostname("movieScreen"); 
  ArduinoOTA.begin();
  MDNS.begin("movieScreen");
  publish("/movieScreen/IP",WiFi.localIP().toString());
}

void loop()
{
  oldA = digitalRead(PIN_A);
  oldA = digitalRead(PIN_B);
  publish( "/movieScreen/A",String( oldA ));
  publish( "/movieScreen/B",String( oldB ));
  
  while (true)
  {
    ArduinoOTA.handle();
    delay(50);

    client.loop();
    if (!client.connected())
    {
      connect();
    }

    int newA = digitalRead( PIN_A );
    int newB = digitalRead( PIN_B );

    if( newA != oldA ){
      oldA = newA;
      publish( "/movieScreen/A",String( newA ));
    }
    if( newB != oldB ){
      oldB = newB;
      //publish( "/movieScreen/B",String( newB ));
      publish( "/movieScreen/position",String( position ));
      if( speed > 0){
        position++;
      }
      else{
        position--;
      }
    }

  }
}