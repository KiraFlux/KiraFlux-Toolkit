#pragma once

#include <unity.h>

void run_tests();

#ifdef ARDUINO

#include <Arduino.h>

__attribute__((weak)) void setup() {
    UNITY_BEGIN();

    run_tests();

    (void) UNITY_END();
}

__attribute__((weak)) void loop() {}

#else

__attribute__((weak)) int main() {
    UNITY_BEGIN();

    run_tests();

    return UNITY_END();
}

#endif