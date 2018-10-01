#include <unity.h>
#include <string>
#ifndef ESP8266
#include <ArduinoFake.h>
#endif
#include "Utils.h"
#include "MessageHandler.h"

using namespace fakeit;

using namespace std;

void test_function_calculator_subtraction(void) {
    string topic = "/ceiling/abc";
    Utils u = Utils();
    MessageHander mh(u);

    TEST_ASSERT_GREATER_OR_EQUAL(1,topic.find("abc"));
    TEST_ASSERT_FALSE(("de"));
    TEST_ASSERT_TRUE(mh.matchesTopic("abc"));
}

void process() {
    UNITY_BEGIN();
    RUN_TEST(test_function_calculator_subtraction);
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