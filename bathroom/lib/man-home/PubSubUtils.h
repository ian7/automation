#define LED_PIN 2

#define LED_ON 0
#define LED_OFF 1

WiFiClient net;
PubSubClient client(net);

void blink(int count)
{
    for (int i = 0; i < count; i++)
    {
        digitalWrite(LED_PIN, LED_ON);
        delay(100);
        digitalWrite(LED_PIN, LED_OFF);
        delay(500);
    }
    delay(1000);
}

void publish(const String &topic, const String &payload)
{
    unsigned char topicChars[100];
    unsigned char payloadChars[100];
    topic.getBytes(topicChars, 100);
    payload.getBytes(payloadChars, 100);
    client.publish((char *)topicChars, (char *)payloadChars); //,false,1);
}

void ack( String message ){
        publish(String("/")+projectName+"/ack", message);
}

void setupOTA()
{
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        { // U_SPIFFS
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
        if (error == OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }
    });

    ArduinoOTA.setHostname(projectName);
    ArduinoOTA.begin();
    MDNS.begin("bathroom");
}