#include "MessageHandler.h"


namespace std
{
}

MessageHander::MessageHander(Utils &utils){
    this->utils = &utils;
}

MessageHander::~MessageHander(){

}

void MessageHander::messageReceived(string topic, string payload){
{
  if( topic.find("/ceiling/abc",0) >= 0 ){
    utils->publish("/ceiling/ack","haha");
  }

  const string topicPrefix = string("/ceiling/set");
  if (topic.find(topicPrefix) >= 0 )
  {
    if( topic.length() <= topicPrefix.length()){
      utils->publish("/ceiling/ack","set topic missing chanel number");
      return;
    }
    utils->publish("/ceiling/ack","set received");
    const int indexOfTopic = topic.find(topicPrefix);
    const int channel = atoi(topic.substr(topicPrefix.length()).c_str());
    const int spaceIndex = payload.find(" ");
    if( spaceIndex <= 0) {
      utils->publish("/ceiling/ack","malformed content, it should contain target _space_ speed");
      return;
    }
    const int target = stoi(payload.substr(0,spaceIndex).c_str());
    const int speed = atoi(payload.substr(spaceIndex+1).c_str());
    utils->publish("/ceiling/ack", "channel: " + to_string(channel) + " target: " + to_string(target) + " speed: " + to_string(speed));
  }
 }
}