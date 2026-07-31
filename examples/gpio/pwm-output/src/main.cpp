// KiraFlux-Toolkit Example 'gpio/pwm-output'

// Demonstrates PWM output generation using the kf::gpio::PwmOutput class.
// The example generates a sawtooth waveform by linearly ramping the duty cycle from 0 to maximum and back,
// and also shows how to set pulse width in microseconds (useful for servo control).

#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: gpio/pwm-output");

    // --- PWM Configuration ---
    // The configuration is stored separately so it can be reused for multiple PWM outputs (e.g., for H-bridge motor drivers).
    // - frequency_hz: PWM frequency in Hertz (10 kHz in this example).
    // - resolution_bits: number of bits for the duty cycle resolution.
    //   The maximum duty value is (2^resolution_bits - 1), i.e., 1023 for 10 bits.
    gpio::PwmOutput::Config my_pwm_config{
        .frequency_hz = 10'000,
        .resolution_bits = 10,
    };

    // Create the PWM output object on GPIO pin 13.
    // The pin number is provided using the gpio::GpioNumber enum class.
    gpio::PwmOutput my_pwm_output{
        my_pwm_config,
        gpio::G13,
    };

    // Initialize the hardware timer (LEDC on ESP32).
    // Returns false if the pin does not support PWM or initialization fails.
    if (not my_pwm_output.init()) {
        init.logger.error("PWM init failed");
        return;
    }

    // Maximum duty cycle value (1023 for 10-bit resolution).
    gpio::PwmOutput::Duty const max_duty = my_pwm_config.maxDuty();

    auto current_duty = 0;

    // Main loop: generates a sawtooth waveform.
    // The duty cycle increases from 0 to max_duty, then resets to 0.
    while (true) {
        // Write the current duty cycle to the PWM output.
        my_pwm_output.write(current_duty);

        // Log the current duty value for monitoring.
        init.logger.info("duty: {}", current_duty);

        // Increment and wrap around.
        current_duty += 1;
        if (current_duty > max_duty) {
            current_duty = 0;
        }

        // Delay 1 ms between steps – determines the ramp speed.
        // Adjust this value to change the sweep rate.
        rtos::Task::sleep(1);
    }

    // Alternative: setting pulse width directly in microseconds.
    // This is convenient for servo motors (typical range: 500–2500 us).
    // Uncomment the line below to set a fixed pulse width (e.g., 500 us).
    // my_pwm_output.writePulse(500);
}