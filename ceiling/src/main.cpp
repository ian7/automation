
#define _GLIBCXX_USE_C99 1

#ifdef ESP8266
#include <Arduino.h>
#endif

#include "Utils.h"
#include "MessageHandler.h"

int pins[9]= {16,5,4,0,2,14,12,13,15};
int speed[9]={0,0,0,0,0,0,0,0,0};
int target[9]={0,0,0,0,0,0,0,0,0};
int level[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};

#include <string>
using namespace std;


Utils utils;
MessageHander mh(utils);

void messageReceived(const string topic, const string payload){
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
