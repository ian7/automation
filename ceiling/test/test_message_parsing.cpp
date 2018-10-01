#include <unity.h>
#include <string>
#ifndef ESP8266
#include <ArduinoFake.h>
#endif
#include "Utils.h"
#include "MessageHandler.h"

using namespace fakeit;
using namespace std;

void test_topic_match(void) {
    string topic = "/ceiling/abc";
    Mock<Utils> utilsMock;
//    When(Method(utilsMock,getProjectName)).Return("hoho");
    Utils &u = utilsMock.get();
    MessageHander mh(u);

    TEST_ASSERT_GREATER_OR_EQUAL(1,topic.find("abc"));
    TEST_ASSERT_TRUE(mh.matchesTopic("abc","/ceiling/abc"));
    TEST_ASSERT_FALSE(mh.matchesTopic("abc","/ceiling/abd"));
    TEST_ASSERT_TRUE(mh.matchesTopic("abc","/ceiling/abc/123"));
    TEST_ASSERT_FALSE(mh.matchesTopic("abc","/ceiling/abd/123"));
}

void test_payload_split(void) {
    Mock<Utils> utilsMock;
//    When(Method(utilsMock,getProjectName)).Return("hoho");
    Utils &u = utilsMock.get();
    MessageHander mh(u);

    TEST_ASSERT_EQUAL_INT(3,mh.split("1 2 3",' ').size());
    TEST_ASSERT_EQUAL_INT(2,mh.split("1 2",' ').size());
}


void process() {
    UNITY_BEGIN();
    RUN_TEST(test_topic_match);
    RUN_TEST(test_payload_split);
    UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);
    process();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}

#else

int main(int argc, char **argv) {
    process();
    return 0;
}

#endif