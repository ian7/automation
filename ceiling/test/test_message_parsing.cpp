#include <unity.h>

void test_function_calculator_subtraction(void) {
    const string topic = "/ceiling/abc"
    
    TEST_ASSERT_EQUAL(20, 20);
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