// KiraFlux-Toolkit Example 'actuator/PwmPositionServo'

#include <kf/arduino/gpio.hpp>
#include <kf/driver/actuator/PwmPositionServo.hpp>
#include <kf/main.hpp>

using kf::arduino::ArduinoPwmOutput;
using PwmPositionServo = kf::driver::actuator::PwmPositionServo<ArduinoPwmOutput>;

// --- Servo configuration ---

// Mapping: angle (0..180°) → pulse width (500..2500 µs)
PwmPositionServo::Config servo_config{
    .angle_range = {.start = 0, .end = 180},
    .pulse_range = {.start = 500, .end = 2500},
};

// PWM output: 50 Hz, 12-bit resolution (standard servo)
ArduinoPwmOutput::Config pwm_config{
    .frequency_hz = 50,
    .resolution_bits = 12,
    .gpio_num = static_cast<kf::u8>(GPIO_NUM_13),
};

// --- Servo instance with safe angle limit (0‑90° instead of 0‑180°) ---

PwmPositionServo servo{
    servo_config,                                               // by const reference
    ArduinoPwmOutput{pwm_config},                               // moved
    PwmPositionServo::Config::AngleRange{.start = 0, .end = 90},// narrow override
};

// --- Main application ---

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: actuator/PwmPositionServo");

    // Initialize the servo (PWM output)
    if (not servo.init()) {
        init.logger.error("Servo init failed");
        return;
    }
    init.logger.info("Servo initialized");

    // Small delay to stabilize hardware
    delay(1000);

    // Sweep from 0° to 90° and back (safe range override)
    // The angle is clamped to 0..90 due to the override limit.
    init.logger.info("Sweeping 0° -> 90° -> 0°");

    for (auto angle = -90; angle <= 90; angle += 1) {
        const auto pos = 90 - abs(angle);// 0 ->  90 ->  0
        servo.set(pos);
        delay(20);
    }

    delay(1000);

    // Stop the servo (disable PWM signal)
    servo.stop();
    init.logger.info("Stopped");
}