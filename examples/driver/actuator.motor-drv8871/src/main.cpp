// KiraFlux-Toolkit Example 'actuator/DRV8871'

#include <kf/arduino/gpio.hpp>
#include <kf/driver/actuator/DRV8871.hpp>
#include <kf/main.hpp>

using kf::arduino::ArduinoPwmOutput;
using DRV8871 = kf::driver::actuator::DRV8871<ArduinoPwmOutput>;

// --- PWM configurations ---

// Forward and backward PWM outputs (must outlive the motor instance)
ArduinoPwmOutput::Config forward_pwm_config{
    .frequency_hz = 30'000,// typical for DC motors
    .resolution_bits = 10,
    .gpio_num = static_cast<kf::u8>(GPIO_NUM_26),
};

ArduinoPwmOutput::Config backward_pwm_config{
    .frequency_hz = 30'000,
    .resolution_bits = 10,
    .gpio_num = static_cast<kf::u8>(GPIO_NUM_25),
};

// --- Motor driver configuration ---

DRV8871::Config driver_config{
    .max_input = 1000,        // max absolute control value
    .forward_dead_zone = 500, // minimum PWM to start moving forward
    .backward_dead_zone = 500,// minimum PWM to start moving backward
};

// --- Motor instance (global/static, outlives the application) ---

DRV8871 motor{
    driver_config,                        // by const reference
    ArduinoPwmOutput{forward_pwm_config}, // moved
    ArduinoPwmOutput{backward_pwm_config},// moved
};

// --- Main application ---

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: actuator/DRV8871");

    // Initialize the motor driver
    if (not motor.init()) {
        init.logger.error("Motor init failed");
        return;
    }
    init.logger.info("Motor initialized");

    // Small delay to stabilize hardware
    delay(1000);

    // Run forward at full speed (max_input = 1000)
    motor.set(+1000);
    init.logger.info("Forward (1000)");
    delay(2000);

    // Run backward at half speed (max_input = 1000, so -500 = 50%)
    motor.set(-500);
    init.logger.info("Backward (500)");
    delay(2000);

    // Stop the motor
    motor.stop();
    init.logger.info("Stopped");
}