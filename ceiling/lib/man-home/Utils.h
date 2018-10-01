/*
 * utils.h
 *
 *  Created on: Oct 1, 2018
 *      Author: Marci
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#ifdef ESP8266

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

#else
#include <ArduinoFake.h>
#endif

#include <wifi-password.h>
#include <string>
using namespace std;

class Utils {
private:
#ifdef ESP8266
	WiFiClient net;
	PubSubClient client;
#endif
	void pubSubCallback(char* topic, char* payload, unsigned int length);

public:
	void publish(const string &topic, const string &payload);
	void connect();
	void loop();
	Utils();
	virtual ~Utils();
};


#ifndef ESP8266
void analogWriteRange(int x);
void analogWriteFreq(int x);
#endif

#endif /* SRC_UTILS_H_ */
