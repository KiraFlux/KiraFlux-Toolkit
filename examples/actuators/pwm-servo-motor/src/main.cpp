#include <Arduino.h>
#include <kf/Logger.hpp>

#include <kf/drivers/actuators/PwmPositionServo.hpp>
#include <kf/gpio/arduino.hpp>

using kf::gpio::arduino::PwmOutput;
using PwmPositionServo = kf::drivers::actuators::PwmPositionServo<PwmOutput>;

// GPIO13, channel 0; 0..180 deg range
static const PwmPositionServo::DriverConfig driver_config{
    .min_angle = 0,
    .max_angle = 180,
};

// Typical 500‑2500 us pulse width for 0‑180 deg
static const PwmPositionServo::PulseConfig pulse_config{
    .min_pulse = {500, 0},
    .max_pulse = {2500, 180},
};

// 50 Hz, 12-bit resolution works for most RC servos
PwmOutput::Config pwm_config{
    .frequency_hz = 50,
    .resolution_bits = 12,
    .pin = static_cast<kf::u8>(GPIO_NUM_13),
    .channel = 0,
};

static PwmPositionServo servo{
    driver_config,
    pulse_config,
    PwmOutput{pwm_config},
};

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView s) { Serial.write(s.data(), s.size()); };

    // check configs
    kf::Validator validator{};
    driver_config.check(validator);
    pulse_config.check(validator);
    if (not validator.passed()) {
        Serial.println("Servo config check failed");
        return;
    }

    if (not servo.init()) {
        Serial.println("Servo init failed");
        return;
    }

    // sweep 0 -> 180 -> 0
    for (int angle = -180; angle <= 180; angle += 1) {
        servo.write(abs(180 - angle));
        delay(20);
    }
}

void loop() {}