// KiraFlux-Toolkit Example 'driver/sensor.quadrature-encoder'

#include <kf/driver/sensor/QuadratureEncoder.hpp>
#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

using kf::driver::sensor::QuadratureEncoder;

// Unit type: we use degrees for position.
using UnitType = float;

// Encoder specialization with DigitalInput GPIO and UnitType.
using Encoder = QuadratureEncoder<UnitType>;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: driver/sensor.quadrature-encoder");

    // --- Configuration (MUST outlive the encoder instance) ---

    // Encoder config: 100 units (degrees) per full tick.
    Encoder::Config encoder_config{
        .units_per_tick = 100,// 100 degrees per tick
        .positive_direction = Encoder::Config::Direction::CW,
        .pull = gpio::DigitalInput::Pull::External,
    };

    // --- Encoder instance ---

    // Config is passed by const reference (must stay alive).
    // GPIO pins are constructed into the encoder.
    Encoder encoder{
        encoder_config,// by const reference
        gpio::G25,     // gpio number phase A
        gpio::G26,     // gpio number phase B
    };

    // Initialize the encoder (sets up GPIO and ISR).
    encoder.init();
    init.logger.info("Encoder initialized");

    // Small delay to let hardware stabilize.
    rtos::Task::sleep(100);

    // --- Show initial position ---

    init.logger.info("Initial position: {} ticks, {} units",
                     encoder.positionTicks(),
                     encoder.positionUnits());

    // --- Main loop: read and log position every 200 ms ---

    while (true) {
        // Accumulated ticks (updated by ISR).
        auto ticks = encoder.positionTicks();

        // Convert ticks to physical units using the config.
        auto units = encoder.positionUnits();

        init.logger.debug("Position: {} ticks, {} units", ticks, units);

        // Wait a bit before next read.
        rtos::Task::sleep(200);

        // Optional: demonstrate resetting position.
        // Uncomment to reset position to zero after some time.
        // static bool reset_done = false;
        // if (not reset_done and ticks > 1000) {
        //     encoder.reset();                        // reset ticks to 0
        //     init.logger.info("Encoder reset");
        //     reset_done = true;
        // }
    }
}