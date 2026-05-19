#include <Arduino.h>
#include <kf/math/Timer.hpp>

kf::math::Timer::Config one_shot_config{.period = 2000};
kf::math::Timer::Config periodic_config{.period = 100}; // 100 ms = 10 Hz
kf::math::Timer::Config stopwatch_config{.period = 0};

kf::math::Timer one_shot{one_shot_config};
kf::math::Timer periodic{periodic_config};
kf::math::Timer stopwatch{stopwatch_config};

bool one_shot_armed{true};

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Timer demo");

    const auto now = millis();
    one_shot.start(now);
    periodic.start(now);
    stopwatch.start(now);
}

void loop() {
    const auto now = millis();

    Serial.print("One-shot remaining: ");
    Serial.println(one_shot.remaining(now));

    if (one_shot_armed and one_shot.expired(now)) {
        one_shot_armed = false;
        Serial.println("One-shot expired");
    }

    if (periodic.expired(now)) {
        Serial.print("Periodic tick at ");
        Serial.println(now);
        periodic.start(now); // re-arm for next period
    }

    Serial.print("Stopwatch: ");
    Serial.println(stopwatch.elapsed(now));

    delay(50);
}