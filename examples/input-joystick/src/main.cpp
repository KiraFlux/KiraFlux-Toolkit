// Demo for KiraFlux input abstractions: AnalogAxis, Joystick, JoystickListener

#include <Arduino.h>
#include <kf/math/Timer.hpp>

// target files for this demo
#include <kf/input/AnalogAxis.hpp>
#include <kf/input/Joystick.hpp>
#include <kf/input/JoystickListener.hpp>

using kf::input::AnalogAxis;
using kf::input::Joystick;
using kf::input::JoystickListener;

// Configuration – must outlive joystick instance (referenced)
Joystick::Config my_joystick_config{
    .x = {
        GPIO_NUM_34,
        AnalogAxis::Config::Mode::Normal,
    },
    .y = {
        GPIO_NUM_35,
        AnalogAxis::Config::Mode::Inverted,
    },
};

// Joystick – must outlive listener instance (referenced)
Joystick my_joystick{
    my_joystick_config,// referenced, must stay alive
    0.1f,              // filter coefficient for both axes
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

// Calibration: AxisTuner measures min, max and computes dead‑zone & ranges.
// Tuners are independent state machines; they can have different sample counts.
void tune(Joystick::Config &config) {
    const unsigned samples_x = 100;
    const unsigned samples_y = 200;// may be different – each tuner stops after its own count

    AnalogAxis::Tuner tunerX{config.x, my_joystick.axis_x, samples_x};
    AnalogAxis::Tuner tunerY{config.y, my_joystick.axis_y, samples_y};

    tunerX.start();
    tunerY.start();

    // Feed samples until both tuners finish
    while (tunerX.running() or tunerY.running()) {
        tunerX.poll();
        tunerY.poll();
    }
    // After this, config.x and config.y contain calibrated dead‑zone and ranges.
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Joystick Listener Demo");

    my_joystick.init();      // sets pin modes
    tune(my_joystick_config);// calibrate – needs GPIO ready, call after init()
}

void loop() {
    static kf::math::Timer log_timer{kf::math::Milliseconds(500)};// print every 500 ms

    const auto now = millis();

    // Periodically log raw and normalised values
    if (log_timer.expired(now)) {
        log_timer.start(now);

        const int x_raw = my_joystick.axis_x.readRaw();
        const float x_norm = my_joystick.axis_x.read();

        const int y_raw = my_joystick.axis_y.readRaw();
        const float y_norm = my_joystick.axis_y.read();

        const auto combined = my_joystick.read();// (x, y, magnitude), normalized to unit circle

        Serial.printf(
            "Ax: %5d %+1.3f \t"
            "Ay: %5d %+1.3f \t"
            "Both: (%+1.3f, %+1.3f): %1.3f\n",
            x_raw, x_norm,
            y_raw, y_norm,
            combined.x, combined.y, combined.magnitude);
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