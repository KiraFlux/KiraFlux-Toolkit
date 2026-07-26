// KiraFlux-Toolkit Example 'driver/sensor.joystick'

#include <kf/Timer.hpp>
#include <kf/driver/sensor/Joystick.hpp>
#include <kf/driver/sensor/NormalizedAdcInput.hpp>
#include <kf/gpio.hpp>
#include <kf/listener/JoystickListener.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Clock.hpp>
#include <kf/rtos/Task.hpp>

using kf::driver::sensor::NormalizedAdcInput;
using Joystick = kf::driver::sensor::Joystick<NormalizedAdcInput>;
using JoystickListener = kf::listener::JoystickListener;
using Direction = JoystickListener::Direction;

// --- Helper: convert direction to string ---

char const *stringFromDirection(Direction dir) {
    switch (dir) {
        case Direction::Center: return "Center";
        case Direction::Up: return "Up";
        case Direction::Down: return "Down";
        case Direction::Left: return "Left";
        case Direction::Right: return "Right";
        default: return "?";
    }
}

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: driver/sensor.joystick");

    // --- Configuration (must outlive the joystick and listener instances) ---
    // These configs are passed by const reference to the Joystick and JoystickListener.
    // They MUST remain valid for the entire lifetime of the joystick and my_listener.

    Joystick::Config joystick_config{
        .x = {
            .inverted = false,
            // dead_zone, range_positive, range_negative are tuned automatically
        },
        .y = {
            .inverted = true,
        },
        .filter = {
            .factor = 0.1f,
        },
    };

    JoystickListener::Config listener_config{
        .repeat_timer = {
            .value = 100,// ms between repeated events
        },
        .delay_timer = {
            .value = 500,// ms delay before first repeat
        },
        .threshold = 0.7f,// neutral zone threshold
    };

    // --- Global instances (must outlive the application) ---
    // The joystick takes ADC inputs by move (moved into the joystick).
    // The joystick configuration is passed by const reference (must stay alive).

    Joystick joystick{
        joystick_config,// by const reference (MUST outlive this instance)
        gpio::G34,      // gpio num for X axis
        gpio::G35,      // gpio num for Y axis
    };

    // The listener takes its configuration by const reference (must stay alive).
    JoystickListener my_listener{
        listener_config,// by const reference (MUST outlive this instance)
    };

    // Initialize hardware (ADC pins). This MUST be called before calibration.
    joystick.init();
    init.logger.info("Joystick initialized");

    // --- Calibration: tune both axes ---
    // The tuner reads raw values from the joystick axes and computes dead zones and ranges.
    // This MUST be called AFTER joystick.init() because GPIO must be ready.
    // Tuner takes config by reference (modifies it) and joystick by reference (reads from it).
    Joystick::Tuner tuner{joystick_config, joystick, /* samples: */ 100};

    init.logger.info("Calibration started...");
    tuner.reset();

    while (tuner.running()) {
        tuner.poll();        // reads X and Y raw values, feeds the tuners
        rtos::Task::sleep(1);// small delay to avoid busy-loop
    }

    init.logger.info("Calibration finished");

    // Set up listener callback.
    // The listener will call this function when direction changes.
    my_listener.callback([&](Direction direction) {
        init.logger.info("Direction: {}", stringFromDirection(direction));
    });

    // Timer for periodic logging of raw values (debug only).
    Timer log_timer{Timer::Config{.value = 500}};
    log_timer.start(millis());

    // --- Main loop ---

    while (true) {
        auto now = millis();

        // Read normalized values (filtered and calibrated).
        float x_norm = joystick.axis_x.read();
        float y_norm = joystick.axis_y.read();

        // Log raw and normalized values every 500 ms (debug).
        if (log_timer.expired(now)) {
            log_timer.start(now);

            int x_raw = joystick.axis_x.readRaw();
            int y_raw = joystick.axis_y.readRaw();

            auto const [x, y, magnitude] = joystick.read();// normalized to unit circle

            init.logger.debug(
                "Ax: {} {}\tAy: {} {}\tBoth: ({}, {}) {}",
                x_raw, x_norm,
                y_raw, y_norm,
                x, y, magnitude);
        }

        // Update listener with the current normalized vector.
        // This triggers direction change events and autorepeat.
        my_listener.set(math::Vector2f::create(x_norm, y_norm));
        my_listener.poll(now);

        // Small delay to avoid flooding the system.
        rtos::Task::sleep(10);
    }
}