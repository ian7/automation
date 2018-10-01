#include "MessageHandler.h"

namespace std
{
}

MessageHander::MessageHander(Utils &utils)
{
    this->utils = &utils;
}

MessageHander::~MessageHander()
{
}

void MessageHander::messageReceived(string topic, string payload)
{
    {
        if (topic.find("/" + this->projectName + "/abc", 0) >= 0)
        {
            utils->publish("/" + this->projectName + "/ack", "haha");
        }

        const string topicPrefix = string("/" + this->projectName + "/set");
        if (topic.find(topicPrefix) >= 0)
        {
            if (topic.length() <= topicPrefix.length())
            {
                utils->publish("/" + this->projectName + "/ack", "set topic missing chanel number");
                return;
            }
            utils->publish("/" + this->projectName + "/ack", "set received");
            const int indexOfTopic = topic.find(topicPrefix);
            const int channel = atoi(topic.substr(topicPrefix.length()).c_str());
            const int spaceIndex = payload.find(" ");
            if (spaceIndex <= 0)
            {
                utils->publish("/" + this->projectName + "/ack", "malformed content, it should contain target _space_ speed");
                return;
            }
            const int target = atoi(payload.substr(0, spaceIndex).c_str());
            const int speed = atoi(payload.substr(spaceIndex + 1).c_str());
            utils->publish("/" + this->projectName + "/ack", "channel: " + to_string(channel) + " target: " + to_string(target) + " speed: " + to_string(speed));
        }
    }
}
bool MessageHander::matchesTopic(string topic, string testee)
{
    return(!(testee.find("/" + this->getProjectName() + "/" + topic) == string::npos));
}

string MessageHander::getProjectName(){
    return this->projectName;
}

template<typename Out> void MessageHander::split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}
std::vector<std::string> MessageHander::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
