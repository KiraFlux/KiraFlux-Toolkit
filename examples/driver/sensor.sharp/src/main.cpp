// KiraFlux-Toolkit Example 'sensor/sharp'

#include <kf/arduino/gpio.hpp>
#include <kf/driver/sensor/Sharp.hpp>
#include <kf/main.hpp>

using kf::arduino::ArduinoAdcInput;
using kf::driver::sensor::Sharp;

// --- Sensor instance ---
// The Sharp sensor takes an ADC input by move.
// The ADC input must be initialized (pin mode) before use.
Sharp distance_sensor{ArduinoAdcInput{GPIO_NUM_30}};

// --- Main application ---

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: sensor/sharp");

    // Set ADC resolution globally (affects all ADC reads).
    // The Sharp sensor formula assumes 10-bit resolution (0..1023).
    ArduinoAdcInput::resolution(10);
    init.logger.debug("ADC resolution set to 10 bits");

    // Initialize the sensor (sets up the ADC pin).
    distance_sensor.init();
    init.logger.info("Sharp sensor initialized");

    // Small delay for hardware stabilization.
    delay(100);

    // --- Main loop: read distance every 50 ms for 10 seconds ---

    for (int i = 0; i < 200; i += 1) {
        // Read distance in millimeters.
        // The sensor uses the ADC value and a conversion formula.
        auto distance_mm = distance_sensor.read();

        init.logger.info("Distance: {} mm", distance_mm);

        // Wait 50 ms between readings.
        delay(50);
    }

    init.logger.info("Demo finished");
}