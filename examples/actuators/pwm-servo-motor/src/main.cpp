#include <Arduino.h>
#include <kf/Logger.hpp>

#include <kf/drivers/actuators/PwmPositionServo.hpp>
#include <kf/gpio/arduino.hpp>

using kf::gpio::arduino::PwmOutput;
using PwmPositionServo = kf::drivers::actuators::PwmPositionServo<PwmOutput>;

// Angle -> pulse width mapping (0° = 500 µs, 180° = 2500 µs)
PwmPositionServo::Config servo_config{
    .angle_range = {.start = 0, .end = 180},
    .pulse_range = {.start = 500, .end = 2500},
};

// PWM: 50 Hz, 12-bit resolution
PwmOutput::Config pwm_config{
    .frequency_hz = 50,
    .resolution_bits = 12,
    .pin = static_cast<kf::u8>(GPIO_NUM_13),
    .channel = 0,
};

// Servo with additional safe angle limit (0‑90 instead of 0‑180)
PwmPositionServo servo{
    servo_config,
    PwmOutput{pwm_config},
    PwmPositionServo::Config::AngleRange{.start = 0, .end = 90},// safe range
};

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView s) { Serial.write(s.data(), s.size()); };

    kf::Validator validator{};
    servo_config.check(validator);
    if (not validator.passed()) {
        Serial.println("Servo config check failed");
        return;
    }

    if (not servo.init()) {
        Serial.println("Servo init failed");
        return;
    }

    // Sweep 0 -> 180 -> 0
    for (int angle = -180; angle <= 180; angle += 1) {
        servo.write(abs(180 - angle));
        delay(20);
    }
}

void loop() {}