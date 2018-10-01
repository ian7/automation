
#define _GLIBCXX_USE_C99 1

#ifdef ESP8266
#include <Arduino.h>
#endif

#include "Utils.h"

int pins[9]= {16,5,4,0,2,14,12,13,15};
int speed[9]={0,0,0,0,0,0,0,0,0};
int target[9]={0,0,0,0,0,0,0,0,0};
int level[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};

#include <string>
using namespace std;


Utils utils;

void messageReceived(const string topic, const string payload)
{
  if( topic.find("/ceiling/abc",0) >= 0 ){
    utils.publish("/ceiling/ack","haha");
  }

  const string topicPrefix = string("/ceiling/set");
  if (topic.find(topicPrefix) >= 0 )
  {
    if( topic.length() <= topicPrefix.length()){
      utils.publish("/ceiling/ack","set topic missing chanel number");
      return;
    }
    utils.publish("/ceiling/ack","set received");
    const int indexOfTopic = topic.find(topicPrefix);
    const int channel = stoi(topic.substr(topicPrefix.length()));
    const int spaceIndex = payload.find(" ");
    if( spaceIndex <= 0) {
      utils.publish("/ceiling/ack","malformed content, it should contain target _space_ speed");
      return;
    }
    const int target = stoi(payload.substr(0,spaceIndex));
    const int speed = stoi(payload.substr(spaceIndex+1));
//    utils.publish("/ceiling/ack", (String("channel: ") + String(channel) + " target: " + String(target) + " speed: " + String(speed)).c_str());
    
  }

}


void setup()
{
   for( int i=0;i<=8;i++){
      pinMode(pins[i], OUTPUT);
    }
    analogWriteRange(1023);
    analogWriteFreq(100);


  
}

void loop()
{
/*
  // simple blinking on D1   
   while(true){
        digitalWrite(5,0);
        delay(500);
        digitalWrite(5,1);
        delay(500);
    }
*/

  while (true)
  {
    delay(10);

    utils.loop();

    for( int i=0;i<=8;i++){
      const int delta = abs( level[i] - target[i]);
      // this catches the last step before reaching the target
      if( level[i] == -1 || delta <= speed[i] ){
        level[i] = target[i];
      }
      else {
        if( level[i] > target[i]){
          level[i] -= speed[i];
        } 
        else{
          level[i] += speed[i];
        }
      }
      analogWrite(pins[i],level[i]);
    }
  }
}
int main(){

}
