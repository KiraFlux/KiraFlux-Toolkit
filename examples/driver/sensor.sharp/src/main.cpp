// KiraFlux-Toolkit Example 'sensor/sharp'

#include <kf/driver/sensor/Sharp.hpp>
#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

using kf::driver::sensor::Sharp;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: sensor/sharp");

    // --- Sensor instance ---
    // The Sharp sensor takes an ADC input by move.
    // The ADC input must be initialized (pin mode) before use.
    Sharp distance_sensor{gpio::G34};

    // Set ADC resolution globally (affects all ADC reads).
    // The Sharp sensor formula assumes 10-bit resolution (0..1023).
    gpio::AdcInput::resolution(10);
    init.logger.debug("ADC resolution set to 10 bits");

    // Initialize the sensor (sets up the ADC pin).
    distance_sensor.init();
    init.logger.info("Sharp sensor initialized");

    // Small delay for hardware stabilization.
    rtos::Task::sleep(100);

    // --- Main loop: read distance every 50 ms for 10 seconds ---

    for (int i = 0; i < 200; i += 1) {
        // Read distance in millimeters.
        // The sensor uses the ADC value and a conversion formula.
        auto const distance_mm = distance_sensor.read();

        init.logger.info("Distance: {} mm", distance_mm);

        // Wait 50 ms between readings.
        rtos::Task::sleep(50);
    }

    init.logger.info("Demo finished");
}