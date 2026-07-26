// KiraFlux-Toolkit Example 'gpio/digital-input'

// Demonstrates reading a digital input pin with configurable pull resistor.
// The example reads the pin state every 100 ms and logs it.

#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: gpio/digital-input");

    // Create a digital input object on GPIO pin 4 with internal pull-down.
    // Pull options: InternalDown (INPUT_PULLDOWN), InternalUp (INPUT_PULLUP), External (INPUT).
    gpio::DigitalInput my_digital_input_pin{
        gpio::G4,
        gpio::DigitalInput::Pull::InternalDown,
    };

    // Initialize the GPIO pin as an input with the specified pull configuration.
    my_digital_input_pin.init();

    // Main loop: read and log the pin state every 100 ms.
    while (true) {
        // Read the current logic level (true = HIGH, false = LOW).
        bool const level = my_digital_input_pin.level();

        // Log the current state.
        init.logger.info("level: {}", level);

        // Wait 100 ms before the next read.
        rtos::Task::sleep(100);
    }
}