// KiraFlux-Toolkit Example 'gpio/adc-input'

// Demonstrates analog-to-digital conversion using kf::gpio::AdcInput.
// The example sets ADC resolution globally, reads the value every 100 ms and logs it.

#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: gpio/adc-input");

    // Create an ADC input object on GPIO pin 35.
    // The pin number is specified using the gpio::GpioNumber enum class.
    gpio::AdcInput my_adc_input{
        gpio::G35,
    };

    // Set the ADC resolution globally (in bits).
    // This affects all ADC readings on the device. Default is 12 bits on ESP32.
    gpio::AdcInput::resolution(12);

    // Query the current resolution (returns the number of bits).
    // gpio::AdcInput::Value is a u16 type representing the raw ADC reading.
    gpio::AdcInput::Value const r = gpio::AdcInput::resolution();

    // Initialize the ADC pin (configures pin mode as INPUT).
    my_adc_input.init();

    // Main loop: read and log the ADC value every 100 ms.
    while (true) {
        // Read the raw analog value (0 .. (2^resolution - 1)).
        auto const value = my_adc_input.read();

        // Log the current value.
        init.logger.info("value: {}", value);

        // Wait 100 ms before the next read.
        rtos::Task::sleep(100);
    }
}