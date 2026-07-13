#include <Arduino.h>
#include <kf/Timer.hpp>

kf::Timer::Config one_shot_config{.value = 2000};
kf::Timer::Config periodic_config{.value = 100};// 100 ms = 10 Hz
kf::Timer::Config stopwatch_config{.value = 0};

kf::Timer one_shot{one_shot_config};
kf::Timer periodic{periodic_config};
kf::Timer stopwatch{stopwatch_config};

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
        periodic.start(now);// re-arm for next period
    }

    Serial.print("Stopwatch: ");
    Serial.println(stopwatch.elapsed(now));

    delay(50);
}