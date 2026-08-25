// KiraFlux-Toolkit Example 'gpio/digital-output'

// Demonstrates basic digital output control using kf::gpio::DigitalOutput.
// The example toggles a GPIO pin every second, logging the current state.

#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: gpio/digital-output");

    // Create a digital output object on GPIO pin 4.
    // The pin number is specified using the gpio::GpioNumber enum class.
    gpio::DigitalOutput my_digital_output_pin{
        gpio::G4,
    };

    // Initialize the GPIO pin as an output.
    // This configures the hardware pin mode (OUTPUT).
    my_digital_output_pin.init();

    bool level = true;

    // Main loop: toggle the output every second.
    while (true) {
        // Write the current logic level (true = HIGH, false = LOW).
        my_digital_output_pin.level(level);

        // Log the current state for monitoring.
        init.logger.info("level: {}", level);

        // Toggle the level for the next iteration.
        level = not level;

        // Wait 1 second before the next toggle.
        rtos::Task::sleep(1000);
    }
}