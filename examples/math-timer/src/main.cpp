#include <Arduino.h>
#include <kf/math/Timer.hpp>

kf::math::Timer one_shot{kf::Milliseconds(2000)};// will fire once after 2s and then be stopped
kf::math::Timer periodic{kf::Hertz(10)};         // fires at 10 Hz, restart ensures continuous rhythm
kf::math::Timer stopwatch{};                     // just measures elapsed time, period is irrelevant

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

    // remaining shows time left before expiration (0 when expired/stopped)
    Serial.print("One-shot remaining: ");
    Serial.println(one_shot.remaining(now));

    if (one_shot.expired(now)) {
        Serial.println("One-shot expired");
        one_shot.stop();// prevent further firing
    }

    if (periodic.expired(now)) {
        Serial.print("Periodic tick at ");
        Serial.println(now);
        periodic.start(now);// re‑arm for next period
    }

    Serial.print("Stopwatch: ");
    Serial.println(stopwatch.elapsed(now));

    delay(50);
}