// KiraFlux-Toolkit Example 'actuator/DRV8871'

#include <kf/driver/actuator/DRV8871.hpp>
#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

using kf::driver::actuator::DRV8871;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: actuator/DRV8871");

    // --- Motor driver configuration (must outlive the motor instance) ---

    DRV8871::Config driver_config{
        .pwm = {
            .frequency_hz = 30'000,
            .resolution_bits = 10,
        },
        .max_input = 1000,    // max absolute control value
        .duty_dead_zone = 500,// minimum PWM to start moving forward
    };

    // --- Motor instance (global/static, outlives the application) ---

    DRV8871 motor{
        driver_config,// by const reference
        gpio::G25,    // forward pin PWM gpio num
        gpio::G26,    // backward pin PWM gpio num
    };

    // Initialize the motor driver
    if (not motor.init()) {
        init.logger.error("Motor init failed");
        return;
    }
    init.logger.info("Motor initialized");

    // Small delay to stabilize hardware
    rtos::Task::sleep(1000);

    // Run forward at full speed (max_input = 1000)
    motor.set(+1000);
    init.logger.info("Forward (1000)");
    rtos::Task::sleep(2000);

    // Run backward at half speed (max_input = 1000, so -500 = 50%)
    motor.set(-500);
    init.logger.info("Backward (500)");
    rtos::Task::sleep(2000);

    // Stop the motor
    motor.stop();
    init.logger.info("Stopped");
}