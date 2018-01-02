#include <Arduino.h>
#include <MQTTClient.h>
#include <WiFi.h>
#include <wifi-password.h>
#include <dht11.h>

dht11 DHT11;
WiFiClient net;
MQTTClient client;


void connect()
{
        while (WiFi.status() != WL_CONNECTED)
        {
                delay(1000);
        }

        while (!client.connect("plantSensors"))
        {
                delay(1000);
        }
        client.subscribe("/plant/sensors");
}

void messageReceived(String &topic, String &payload)
{

        if (topic == String("/plant/sensors")) {
                if( payload == String("poll")){
                        digitalWrite(25,1);
                        client.publish("/plant/moisture/1",String(analogRead(36)));
                        client.publish("/plant/moisture/2",String(analogRead(39)));
                        client.publish("/plant/moisture/3",String(analogRead(34)));
                        
                        int chk = DHT11.read(26);
                        client.publish("/plant/temperature",String((float)DHT11.temperature));
                        client.publish("/plant/humidity",String((float)DHT11.humidity));
                        //Serial.println(DHT.humidity);
                        delay(50);
                        digitalWrite(25,0);
                }

                client.publish("/plant/ack", topic + "" + payload);
        }
}

void setup()
{
        WiFi.begin(ssid, pass);
        client.begin("10.10.4.1", net);
        client.onMessage(messageReceived);
        connect();

        analogReadResolution(11);
        analogSetAttenuation(ADC_6db); 
        pinMode(39,INPUT);
        pinMode(36,INPUT);
        pinMode(34,INPUT);
        pinMode(25,OUTPUT);
        pinMode(26,INPUT);

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

        delay(10);
        }
}
