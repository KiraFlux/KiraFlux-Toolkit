// Demo for KiraFlux input abstractions: NormalizedAdcInput, Joystick, JoystickListener

#include <Arduino.h>
#include <kf/Timer.hpp>

// target files for this demo
#include <kf/arduino/gpio.hpp>
#include <kf/driver/sensor/Joystick.hpp>
#include <kf/driver/sensor/NormalizedAdcInput.hpp>

#include <kf/listener/JoystickListener.hpp>

using AdcInput = kf::arduino::ArduinoAdcInput;
using NormalizedAdcInput = kf::driver::sensor::NormalizedAdcInput<AdcInput>;
using Joystick = kf::driver::sensor::Joystick<NormalizedAdcInput>;
using JoystickListener = kf::listener::JoystickListener;

// Configuration – must outlive joystick instance (referenced)
Joystick::Config my_joystick_config{
    .x = {
        .inverted = false,

        // tuned automatically:
        // .dead_zone = ... ,
        // .range_positive = ... ,
        // .range_negative = ... ,
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
    my_joystick_config,   // referenced, must stay alive
    axis_filter_config,   // referenced, must stay alive
    AdcInput{GPIO_NUM_34},// moved
    AdcInput{GPIO_NUM_35},// moved
};

JoystickListener::Config my_listener_config{
    .repeat_timer = {
        .value = 100,// ms
    },
    .delay_timer = {
        .value = 500,// ms
    },
    .threshold = 0.7f,// neutral zone threshold (normalized value)
};

JoystickListener my_listener{
    my_listener_config,// referenced, must stay alive
};

using Direction = JoystickListener::Direction;

const char *stringFromDirection(Direction dir) {
    switch (dir) {
        case Direction::Center: return "Center";
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

    Joystick::Tuner tuner{config, my_joystick, samples};

    tuner.reset();

    while (tuner.running()) {
        tuner.poll();// reads X and Y raw values and feeds the tuners
        delay(1);
    }
}

static kf::Timer::Config log_timer_config{
    .value = 500,// print every 500 ms
};

static kf::Timer log_timer{log_timer_config};

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Joystick Listener Demo");

    my_joystick.init();      // sets pin modes
    tune(my_joystick_config);// calibrate – needs GPIO ready, call after init()
    log_timer.start(millis());

    my_listener.callback([](JoystickListener::Direction direction) {
        Serial.println(stringFromDirection(direction));
    });
}

void loop() {
    const auto now = millis();

    const float x_norm = my_joystick.axis_x.read();
    const float y_norm = my_joystick.axis_y.read();

    // Periodically log raw and normalized values
    if (log_timer.expired(now)) {
        log_timer.start(now);

        const int x_raw = my_joystick.axis_x.readRaw();
        const int y_raw = my_joystick.axis_y.readRaw();

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
    my_listener.set(kf::math::Vector2f{.x = x_norm, .y = y_norm});
    my_listener.poll(now);

    delay(10);
}