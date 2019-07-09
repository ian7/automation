#include <Arduino.h>
#include <ArduinoOTA.h>

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "wifi-password.h"
#include <EEPROM.h>
#include <list>
#include "PCF8574.h"

WiFiClient net;
PubSubClient client(net);
PCF8574 register1(0x38,5,4);

uint16_t RECV_PIN = 23;

// How many leds in your strip?
#define NUM_LEDS 150

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 5
#define CLOCK_PIN 2

#define PROGRAM_ADDRESS 500

#define PROGRAM_STATIC 0
#define PROGRAM_TWINLE 1


void registerInterrupt();


void blink( int times = 1, int length = 50 );
void reportSettings();

void publish(const String &topic, const String &payload)
{
    unsigned char topicChars[200];
    unsigned char payloadChars[200];
    topic.getBytes(topicChars, 200);
    payload.getBytes(payloadChars, 200);
    client.publish((char *)topicChars, (char *)payloadChars); //,false,1);
}

class Timer{
  private:
    long int stamp;
    long int time;
  public:
    Timer(long int time =-1 ){
        setTime( time );
        start();
    }
    void setTime( long int time ){
      this->time = time; 
    }
    void start( long int time = -1){
      if( time != -1 ){
        setTime( time );
      }
      this->stamp = millis();
    }
    boolean isElapsed(){
      const long int now = millis();
      const long int threshold = this->stamp + this->time;
      return( now > threshold );
    }
};

Timer watchdogTimer(10000);

class SavedValue { 
  protected: 
    int address;
    int value;
  public:
    SavedValue( int address ){
      this->address = address;
      this->value = EEPROM.read(address);
    }
    int get(){
      return value;
    }
    void set(int value){
      this->value = value;
      EEPROM.write(this->address,value);
      EEPROM.commit();
    }
};

class Setting : public SavedValue {
  private:
    String topic;
    double scale;
  public:
    Setting( int address, String topic, double scale = 1) 
    : SavedValue( address ){
      this->topic = topic;
      this->scale = scale;
    }
    int get(){
      return SavedValue::get()*scale;
    }
    double getDouble(){
      const double doubleValue = (double)SavedValue::get();
      return doubleValue*scale;
    }
    void set(int value){
      SavedValue::set(value/scale);
    }
    void setFloat( float value ){
      SavedValue::set((int) (value/scale));
    }
    String getTopic(){
      return topic;
    }
};




std::list<Setting *> settings;
Setting * activeProgram;
Setting * debugLevel;


// this is defined in wifi-password.h
//const char ssid[] = "xxx";
//const char pass[] = "xxx";

// Define the array of leds

void messageReceived(const String topic, const String payload)
{
  if (topic == String("/watering3/test")){
      publish("/light/tosia/ack", "test");
  }

  if (topic == String("/heartbeat")){
      //publish("/light/tosia/ack", "heartbeat");
      watchdogTimer.start();
  }


  // let's iterate over settings
  std::list<Setting *>::iterator it;
  for (it = settings.begin(); it != settings.end(); it++)
  {
    if( topic == (*it)->getTopic()){
      (*it)->setFloat(payload.toFloat());
      publish("/light/tosia/ack", "topic: " + topic + " payload: " + payload);
      blink();
    }  
  }

  if (topic == String("/watering3/reportSettings"))
  {
    reportSettings();
  }

  if (topic == String("/watering3/reset"))
  {
    ESP.restart();
  }

  if (topic == String("/watering3/register1")){
      publish("/light/tosia/ack", "register1-high");
      register1.digitalWrite(P1,HIGH);
  }
  if (topic == String("/watering3/register1l")){
      publish("/light/tosia/ack", "register1-low");
      register1.digitalWrite(P1,LOW);
  }


}

void pubSubCallback(char* topic, byte* payload, unsigned int length){
    String payloadString = String((char*)payload).substring(0,length);
    String topicString = String(topic);
    messageReceived(topicString, payloadString);
}


void connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  client.setServer("10.10.4.1",1883);
  client.setCallback(pubSubCallback);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    //ESP.restart();
  }
  while (!client.connect("watering3"))
  {
    delay(100);
  }
  client.subscribe("/watering3/#");
  client.subscribe("/heartbeat/#");
  client.subscribe("/heartbeat");
}

#define LED 2

void blink( int times = 1, int length = 50 ){
  for( int i=0; i<times; i++ ){
    delay(length);
    digitalWrite(LED,LOW);
    delay(length);
    digitalWrite(LED,HIGH);
  }
}



void reportSettings(){
    std::list<Setting *>::iterator it;
    for (it = settings.begin(); it != settings.end(); it++)
    {
        publish("/watering3/savedSetting", "topic: " + (*it)->getTopic() + " payload: " + (*it)->getDouble());
        blink();
    }
}

void setup()
{
  pinMode(LED,OUTPUT);
  blink();

  connect();
  blink(2);

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

    ArduinoOTA.setHostname("watering3");
    ArduinoOTA.begin();

    publish("/watering3/IP", WiFi.localIP().toString());
    publish("/watering3/RSSI", String(WiFi.RSSI()));
    publish("/watering3/version", "1");

    // EEPROM.begin(512);

    activeProgram = new Setting(PROGRAM_ADDRESS,"/watering3/program");
    debugLevel = new Setting(PROGRAM_ADDRESS,"/watering3/debugLevel");

    settings.push_back(activeProgram);
    settings.push_back(debugLevel);

  // report on settings
    reportSettings();
  register1.pinMode(P0,OUTPUT);
  register1.pinMode(P1,OUTPUT);
  register1.pinMode(P2,OUTPUT);
  register1.pinMode(P3,OUTPUT);
  register1.pinMode(P4,OUTPUT);
  register1.pinMode(P5,OUTPUT);
  register1.pinMode(P6,OUTPUT);
  register1.pinMode(P7,OUTPUT);
  register1.begin();

  blink(4);
}


long int lastUpdate = 0;

boolean isElapsed( long int duration ){
  const long int elapsed = millis() - lastUpdate;
  return elapsed > duration; 
}

void update(){
  lastUpdate = millis();
}

void loop()
{
  boolean blinkState = false;
  lastUpdate = millis();

  Timer debugTimer(2000);
  Timer blinkTimer(2000);
  // 10 seconds to get it

  while( true ) {
    client.loop();
    if (!client.connected())
    {
      connect();
    }
    
   if( debugLevel->get() > 0 && debugTimer.isElapsed() ){
     publish("/watering3/debug","debug");
     debugTimer.start();
   }

   if( blinkTimer.isElapsed() ){
     publish("/watering3/blink",String(blinkState));
     blinkState = !blinkState;
     if( blinkState ){
      register1.digitalWrite(P0,LOW);
      publish("/watering3/blink","low");
     }
    else {
      register1.digitalWrite(P0,HIGH);
      publish("/watering3/blink","high");
     }
     blinkTimer.start();
   }

    ArduinoOTA.handle();

    if( watchdogTimer.isElapsed() ){
      publish("/watering3/debug","watchdog triggered");
      ESP.reset();
    }

    // do not remove
    // otherwise whole thing stops working
    delay(10);
  }
}

void registerInterrupt(){
	// Interrupt called (No Serial no read no wire in this function, and DEBUG disabled on PCF library)
	 keyPressed = true;

}