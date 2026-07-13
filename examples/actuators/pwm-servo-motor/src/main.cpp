#include <Arduino.h>
#include <kf/Logger.hpp>

#include <kf/arduino/ArduinoGPIO.hpp>
#include <kf/driver/actuator/PwmPositionServo.hpp>

using PwmOutput = kf::arduino::ArduinoGPIO::PwmOutput;
using PwmPositionServo = kf::driver::actuator::PwmPositionServo<PwmOutput>;

// Angle -> pulse width mapping (0° = 500 µs, 180° = 2500 µs)
PwmPositionServo::Config servo_config{
    .angle_range = {.start = 0, .end = 180},
    .pulse_range = {.start = 500, .end = 2500},
};

// PWM: 50 Hz, 12-bit resolution
PwmOutput::Config pwm_config{
    .frequency_hz = 50,
    .resolution_bits = 12,
    .gpio_num = static_cast<kf::u8>(GPIO_NUM_13),
};

// Servo with additional safe angle limit (0‑90 instead of 0‑180)
PwmPositionServo servo{
    servo_config,
    PwmOutput{pwm_config},
    PwmPositionServo::Config::AngleRange{.start = 0, .end = 90},// narrow override of available position range
};

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = [](kf::StringView s) { Serial.write(s.data(), s.length()); };

    if (not servo.init()) {
        Serial.println("Servo init failed");
        return;
    }

    // Sweep 0 -> 90 -> 0 (NARROW RANGE OVERRIDE)
    for (int angle = -90; angle <= 90; angle += 1) {
        servo.write(abs(90 - angle));
        delay(20);
    }
}

void loop() {}