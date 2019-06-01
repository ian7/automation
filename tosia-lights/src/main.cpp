#include <Arduino.h>
#include <ArduinoOTA.h>

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER

#include <FastLED.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "wifi-password.h"
#include <EEPROM.h>

WiFiClient net;
PubSubClient client(net);

uint16_t RECV_PIN = 23;

// How many leds in your strip?
#define NUM_LEDS 150

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 5
#define CLOCK_PIN 2

int red = 10;
int green = 10;
int blue = 10;

int hue = 10;
int saturation = 10;
int brightness = 10;

int oldHue = hue;
int oldSaturation = saturation;
int oldBrightness = brightness;

int oldRed = red;
int oldGreen = green;
int oldBlue = blue;

void blink( int times = 1, int length = 50 );

void publish(const String &topic, const String &payload)
{
    unsigned char topicChars[200];
    unsigned char payloadChars[200];
    topic.getBytes(topicChars, 200);
    payload.getBytes(payloadChars, 200);
    client.publish((char *)topicChars, (char *)payloadChars); //,false,1);
}

class HSB{
  public:
    double h;
    double s;
    double b;
    HSB(){
      this->h = 1;
      this->s = 0;
      this->b = 0;
    }
    boolean isEqual(HSB &that){
      if( (int) h != (int) that.h || 
          (int) s != (int) that.s ||
          (int) b != (int) that.b ){
            return false;
          }
          else{
            return true;
          }
    }
    int sign( int a, int b ){
      if( a == b ){
        return 0;
      }
      if( a > b ){
        return 1;
      }
      else{
        return -1;
      }
    }
    boolean step( const HSB &dst, double speed=0 ){
      if( dst.h == h && dst.s == s && dst.b == b ){
        return false;
      }
      if( speed == 0 ){
        h = dst.h;
        s = dst.s;
        b = dst.b;
      }
      else{
        this->h -= sign( this->h, dst.h)*speed;
        this->s -= sign( this->s, dst.s)*speed;
        this->b -= sign( this->b, dst.b)*speed;
      }
      return true;
    }
    void black(){
      b=0;
      s=0;
      h=0;
    }
    void white(){
      b=255;
      s=0;
      h=0;
    }
    String toString(){
      return "h: " + String(h) + " s: " + String(s) +" b: " + String(b);
    }
    CHSV toColor(){
      return CHSV((int)h, (int)s, (int)b);
    }
    int countSpaces(String s){
      int i = s.indexOf(" ");
      int count = 0;
      while( i!=-1 ){
        count++;
        i=s.indexOf(" ",i+1);
      }
      return count;
    }
    bool parseString(String input){
      if( countSpaces(input) != 2 ){
        return false;
      }
      int firstSpace = input.indexOf(" ");
      int secondSpace = input.indexOf(" ",firstSpace+1);
      h = input.substring(0,firstSpace).toInt();
      s = input.substring(firstSpace+1,secondSpace).toInt();
      b = input.substring(secondSpace+1).toInt();
      //publish("/light/tosia/ack/parse", toString());
      return true;
    }
};

HSB now[NUM_LEDS];
HSB dst[NUM_LEDS];
double speed[NUM_LEDS]; 
long int lastChanged[NUM_LEDS];


// this is defined in wifi-password.h
//const char ssid[] = "xxx";
//const char pass[] = "xxx";

// Define the array of leds
CRGB leds[NUM_LEDS];

void messageReceived(const String topic, const String payload)
{
  if (topic == String("/tosia-lights/black")){
      for( int i=0;i<NUM_LEDS;i++){
        dst[i].black();
        speed[i]=0.7;
      }
    blink();
    publish("/light/tosia/ack", dst[13].toString());
  }
  if (topic == String("/tosia-lights/white")){
      for( int i=0;i<NUM_LEDS;i++){
        dst[i].white();
      }
    blink();
    publish("/light/tosia/ack", dst[13].toString());
  }
  if (topic == String("/tosia-lights/all")){
      for( int i=0;i<NUM_LEDS;i++){
        dst[i].parseString(payload);
        speed[i]=0.7;
      }
    blink();
    publish("/light/tosia/ack", dst[13].toString());
  }

  if (topic.indexOf("/tosia-lights/fadeAll")==0){
      for( int i=0;i<NUM_LEDS;i++){
        dst[i].b=0;
        speed[i]=0.6;
      }
    blink();
//    publish("/light/tosia/ack", dst[13].toString());
  }

  if (topic.indexOf("/tosia-lights/set")==0){
    int lastSlash = topic.lastIndexOf("/");
    if( lastSlash == topic.length()-1){
      publish("/light/tosia/ack","missing led id");
      return;
    }
    int i = topic.substring( lastSlash+1 ).toInt();
    dst[i].parseString(payload);
    speed[i]=0;
    blink();
//    publish("/light/tosia/ack", dst[i].toString());
  }

  if (topic.indexOf("/tosia-lights/fade/")==0){
    int lastSlash = topic.lastIndexOf("/");
    if( lastSlash == topic.length()-1){
      publish("/light/tosia/ack","missing led id");
      return;
    }
    int i = topic.substring( lastSlash+1 ).toInt();
    dst[i].parseString(payload);
    speed[i]=0.3;
    //publish("/light/tosia/ack", dst[i].toString());
    blink();
  }

  if (topic == String("/tosia-lights/hue"))
  {
    int hue = payload.toInt();
    for( int i=0;i<NUM_LEDS;i++){
        dst[i].h=hue;
        speed[i]=0.8;
      }
    publish("/light/tosia/ack", "h: " + String(hue) + " s: " + String(saturation) + " b: " + String(brightness));
  }
  if (topic == String("/tosia-lights/saturation"))
  {
    int saturation = payload.toInt();
    for( int i=0;i<NUM_LEDS;i++){
        dst[i].s=saturation;
        speed[i]=0.8;
    }

    publish("/light/tosia/ack", "h: " + String(hue) + " s: " + String(saturation) + " b: " + String(brightness));
  }
  if (topic == String("/light/tosia/brightness"))
  {
    int brightness = payload.toInt();
    for( int i=0;i<NUM_LEDS;i++){
        dst[i].b=brightness;
        speed[i]=0.8;
    }

    publish("/light/tosia/ack", "h: " + String(hue) + " s: " + String(saturation) + " b: " + String(brightness));
  }

  return;

  if (topic == String("/light/tosia/red"))
  {
    red = payload.toInt();
    publish("/light/tosia/ack", "red " + payload);
  }
  if (topic == String("/light/tosia/green"))
  {
    red = payload.toInt();
    publish("/light/tosia/ack", "green " + payload);
  }
  if (topic == String("/light/tosia/blue"))
  {
    red = payload.toInt();
    publish("/light/tosia/ack", "blue " + payload);
  }
  //show();
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
    delay(200);
    //ESP.restart();
  }
  while (!client.connect("tosia-lights"))
  {
    delay(200);
  }
  client.subscribe("/tosia-lights/#");
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


void setup()
{
  pinMode(LED,OUTPUT);
  LEDS.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS, 0);
  LEDS.setBrightness(250);
  blink();

  //client.begin("10.10.4.1", net);
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

    ArduinoOTA.setHostname("tosia-light");
    ArduinoOTA.begin();

/*
  for( int j=0; j<2*256;j++){
      for (int i = 0; i < NUM_LEDS; i++)
      {
        // Set the i'th led to red
        CHSV newColor = CHSV((i+j)%255, 255, 255);
        hsv2rgb_rainbow(newColor, leds[i]);
        // Show the leds
      }
      j++;
      FastLED.show();
  }
  for( int i=0;i<NUM_LEDS;i++){
        dst[i].black();
  }
*/
  EEPROM.begin(512);

  for( int i=0; i<NUM_LEDS;i++){
    const long int timeNow = millis();
    lastChanged[i]=timeNow;
    speed[i]=0;

    const int addressBase = i*3;
    dst[i].h = EEPROM.read(addressBase);
    dst[i].s = EEPROM.read(addressBase+1);
    dst[i].b = EEPROM.read(addressBase+2);
  }

  publish("/tosia-lights/IP", WiFi.localIP().toString());
  //publish("/tosia-lights/RSSI", String(WiFi.RSSI()));
  publish("/tosia-lights/version", "6");
  blink(4);
}


void loop()
{
  while( true ) {
    client.loop();
    /*if (!client.connected())
    {
      connect();
    }
    */
   const long int timeNow = millis();
    for( int i=0; i<NUM_LEDS;i++){
      // in case stuff is different
      if( !now[i].isEqual(dst[i]) ){
        // if the step really changes something 
        if( now[i].step(dst[i],speed[i]) ){
          lastChanged[i] = timeNow;
        }
        hsv2rgb_rainbow(now[i].toColor(), leds[i]);
      }
    }
    FastLED.show();
    delay(2);

    bool somethingChanged = false;
    for( int i=0; i<NUM_LEDS;i++){
      if( timeNow - lastChanged[i] > 3000 ){
        somethingChanged = true;
        lastChanged[i] = timeNow;
        const int baseAddress = i*3;
        EEPROM.write(baseAddress,now[i].h);
        EEPROM.write(baseAddress+1,now[i].s);
        EEPROM.write(baseAddress+2,now[i].b);
      }
    }

    if(somethingChanged){
        EEPROM.commit();
    }

    ArduinoOTA.handle();

/*    if (oldHue != hue || oldSaturation != saturation || oldBrightness != brightness)
    {
      oldHue = hue;
      oldBrightness = brightness;
      oldSaturation = saturation;

      // First slide the led in one direction
      for (int i = 0; i < NUM_LEDS; i++)
      {
        // Set the i'th led to red
        CHSV newColor = CHSV(oldHue, oldSaturation, oldBrightness);
        hsv2rgb_rainbow(newColor, leds[i]);
        // Show the leds
      }
      FastLED.show();
    }
*/
/*    if (irrecv.decode(&results))
    {
      if (results.decode_type == NEC)
      {
        uint64_t number = results.value;
        unsigned long long1 = (unsigned long)((number & 0xFFFF0000) >> 16);
        unsigned long long2 = (unsigned long)((number & 0x0000FFFF));

        String hex = String(long1, HEX) + String(long2, HEX); // six octets

        publish("/light/tosia/irReceiver", hex);
      }

      irrecv.resume(); // Receive the next value
    }
*/
  }
}