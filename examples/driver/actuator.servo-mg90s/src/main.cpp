// KiraFlux-Toolkit Example 'actuator/PwmPositionServo'

#include <kf/driver/actuator/PwmPositionServo.hpp>
#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

using kf::driver::actuator::PwmPositionServo;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: actuator/PwmPositionServo");

    // --- Servo configuration ---

    PwmPositionServo::Config servo_config{

        // PWM output: 50 Hz, 12-bit resolution (standard servo)
        .pwm = {
            .frequency_hz = 50,
            .resolution_bits = 12,
        },

        // Mapping: angle (0..180) -> pulse width (500..2500 us)
        .angle_range = {.start = 0, .end = 180},
        .pulse_range = {.start = 500, .end = 2500},
    };

    // --- Servo instance with safe angle limit (0‑90 instead of 0‑180) ---

    PwmPositionServo servo{
        servo_config,                                               // by const reference
        gpio::G13,                                                  // GPIO should support PWM
        PwmPositionServo::Config::AngleRange{.start = 0, .end = 90},// (optional) narrow override
    };

    // Initialize the servo (PWM output)
    if (not servo.init()) {
        init.logger.error("Servo init failed");
        return;
    }

    init.logger.info("Servo initialized");

    // Small delay to stabilize hardware
    rtos::Task::sleep(1000);

    // Sweep from 0 to 90 and back (safe range override)
    // The angle is clamped to 0..90 due to the override limit.
    init.logger.info("Sweeping 0 -> 90 -> 0");

    for (auto angle = 0; angle <= 90; angle += 1) {
        servo.set(angle);
        rtos::Task::sleep(20);
    }

    rtos::Task::sleep(1000);

    // Stop the servo (disable PWM signal)
    servo.stop();
    init.logger.info("Stopped");
}