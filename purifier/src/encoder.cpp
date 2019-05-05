#ifdef ESP8266
#include <Arduino.h>
#else
//#include<fake
#endif

#include "encoder.h"

    Encoder::Encoder(int primaryPin, int secondaryPin){
        pinMode(primaryPin, INPUT);
        digitalWrite(primaryPin,0);
        pinMode(secondaryPin, INPUT);
        digitalWrite(secondaryPin,0);
        this->primaryPin = primaryPin;
        this->secondaryPin = secondaryPin;
        this->lastState = digitalRead(primaryPin);
        this->position = 0;
    }
    Encoder::Encoder(int primaryPin, int secondaryPin, int min, int max){
        pinMode(primaryPin, INPUT);
        digitalWrite(primaryPin,0);
        pinMode(secondaryPin, INPUT);
        digitalWrite(secondaryPin,0);
        this->primaryPin = primaryPin;
        this->secondaryPin = secondaryPin;
        this->lastState = digitalRead(primaryPin);
        this->min = min;
        this->max = max;
        this->position = min;
    }

    bool Encoder::check(){
        const bool newState = digitalRead(primaryPin);
        if( newState != this->lastState){
            this->lastState = newState;
            const bool direction = digitalRead(secondaryPin);
            if( direction==newState ){
                this->position++;
            }
            else{
                this->position--;
            }
            if( min != -1 && max != -1 ){
                if( position > max ){
                    position = max;
                }
                if( position < min ){
                    position = min;
                }
            }
            return true;
        }
        return false;
    }
    int Encoder::getPosition(){
        return this->position;
    }
