/*
 * utils.cpp
 *
 *  Created on: Oct 1, 2018
 *      Author: Marci
 */

#include "Utils.h"

Utils::Utils() {
	// TODO Auto-generated constructor stub
}

void Utils::publish(const string &topic, const string &payload) {
#ifdef ESP8266
	client.publish((char *) topic.c_str(), (char *) payload.c_str());
#endif
}

void Utils::connect() {
#ifdef ESP8266

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pass);

	client.setServer("10.10.4.1", 1883);
	//client.setCallback(utils::pubSubCallback);

	while (WiFi.status() != WL_CONNECTED) {
		delay(200);
	}
	while (!client.connect("ceiling")) {
		delay(200);
	}
	client.subscribe("/ceiling/+");
#endif
}

void Utils::pubSubCallback(char* topic, char* payload, unsigned int length) {
	//string payloadstring = string((char*) payload).substring(0, length);
	string topicstring = string(topic);
	//messageReceived(topicstring, payloadstring);
}

void Utils::loop() {
#ifdef ESP8266
	if (!client.connected()) {
		connect();
	}
	client.loop();
#endif
}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}

#ifndef ESP8266
	void analogWriteRange(int x){

	}
	void analogWriteFreq(int x){

	}
	void analogWrite(int x, int y){

	}
	void pinMode(int x, int y){

	}

	void delay( int x ){

	}
	int abs( int x ){
		if( x > 0 ){
			return x;
		}
		else{
			return -x;
		}
	}

#endif

