// Demo for KiraFlux input abstractions: NormalizedAdcInput, Joystick, JoystickListener

#include <Arduino.h>
#include <kf/math/Timer.hpp>

// target files for this demo
#include <kf/gpio/arduino.hpp>
#include <kf/input/Joystick.hpp>
#include <kf/input/JoystickListener.hpp>
#include <kf/input/NormalizedAdcInput.hpp>

using kf::gpio::arduino::AdcInput;
using NormalizedAdcInput = kf::input::NormalizedAdcInput<AdcInput>;
using Joystick = kf::input::Joystick<NormalizedAdcInput>;
using JoystickListener = kf::input::JoystickListener<Joystick>;

// Configuration – must outlive joystick instance (referenced)
Joystick::Config my_joystick_config{
    .x = {
        .inverted = false,
        // .dead_zone = 123,
        // .range_positive = 45,
        // .range_negative = 67,
    },
    .y = {
        .inverted = true,
    },
};

NormalizedAdcInput::FilterImpl::Config axis_filter_config{
    .factor = 0.1f,
};

// Joystick – must outlive listener instance (referenced)
Joystick my_joystick{
    my_joystick_config,// referenced, must stay alive
    axis_filter_config,// referenced, must stay alive
    AdcInput{GPIO_NUM_34},
    AdcInput{GPIO_NUM_35},
};

JoystickListener my_listener{
    my_joystick,// referenced, must stay alive
    0.5f,       // neutral zone threshold (normalized value)
};

using Direction = JoystickListener::Direction;

const char *stringFromDirection(Direction dir) {
    switch (dir) {
        case Direction::Home: return "Center";
        case Direction::Up: return "Up";
        case Direction::Down: return "Down";
        case Direction::Left: return "Left";
        case Direction::Right: return "Right";
    }
    return "?";// unreachable, but silences warning
}

// Calibration: create a combined tuner for both axes.
// The tuner internally uses AxisTuner for X and Y; it reads raw values from
// the joystick axes automatically on each poll().
void tune(Joystick::Config &config) {
    const unsigned samples = 100;
    auto tuner{config.createTuner(my_joystick, samples)};

    tuner.start();

    while (tuner.running()) {
        tuner.poll();// reads X and Y raw values and feeds the tuners
        delay(1);
    }
}

static kf::math::Timer log_timer{kf::math::Milliseconds(500)};// print every 500 ms

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Joystick Listener Demo");

    my_joystick.init();      // sets pin modes
    tune(my_joystick_config);// calibrate – needs GPIO ready, call after init()
    log_timer.start(millis());
}

void loop() {
    const auto now = millis();

    // Periodically log raw and normalised values
    if (log_timer.expired(now)) {
        log_timer.start(now);

        const int x_raw = my_joystick.axis_x.readRaw();
        const float x_norm = my_joystick.axis_x.read();

        const int y_raw = my_joystick.axis_y.readRaw();
        const float y_norm = my_joystick.axis_y.read();

        const auto [x, y, magnitude] = my_joystick.read();// normalized to unit circle

        Serial.printf(
            "Ax: %5d %+1.3f \t"
            "Ay: %5d %+1.3f \t"
            "Both: (%+1.3f, %+1.3f): %1.3f\n",
            x_raw, x_norm,
            y_raw, y_norm,
            x, y, magnitude);
    }

    // Direction change events (with autorepeat)
    my_listener.poll(now);

    if (my_listener.changed()) {
        Serial.print(stringFromDirection(my_listener.direction()));
        if (my_listener.repeating()) {
            Serial.print(" (repeat)");
        }
        Serial.println();
    }

    delay(10);
}