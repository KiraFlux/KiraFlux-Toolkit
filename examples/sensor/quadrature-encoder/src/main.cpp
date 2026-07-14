// KiraFlux-Toolkit Demo 'quadrature-encoder'

#include <Arduino.h>

#include <kf/arduino/ArduinoGPIO.hpp>
#include <kf/driver/sensor/QuadratureEncoder.hpp>

// This examples uses Arduino Environment and Toolkit's Arduino GPIO Implementation for simplicity
using DigitalInput = kf::arduino::ArduinoGPIO::DigitalInput;

// For example, define Degrees as float alias
using MyDegreesUnit = float;

// Specialization
using QuadratureEncoder = kf::driver::sensor::QuadratureEncoder<
    DigitalInput,// GPIO Implementation
    MyDegreesUnit// Position Unit type
    >;

// Encoder configuration. Must outlive any encoder instance.
QuadratureEncoder::Config encoder_config{
    .units_per_tick = 100,// 100 units = 1 encoder tick
    .positive_direction = QuadratureEncoder::Config::Direction::CW,
};

QuadratureEncoder encoder{
    encoder_config,                                             // by reference (Config MUST OUTLIVE this instance)
    DigitalInput{GPIO_NUM_25, DigitalInput::Pull::ExternalDown},// moved
    DigitalInput{GPIO_NUM_26, DigitalInput::Pull::ExternalDown},// moved
};

void setup() {
    Serial.begin(115200);

    // Initialize sensor (init phase A/B GPIOs)
    encoder.init();
}

void loop() {

    // const auto encoder_read = encoder.read(); // read phase state as u32 ((A << 1) | B)

    // Get

    // get all accumulated ticks
    const auto position_ticks = encoder.positionTicks();

    // calculate units equivalent from actual ticks value
    const auto position_units = encoder.positionUnits();

    // Set

    // set accumulated ticks as 0
    // encoder.positionTicks(0);

    // set accumulated ticks from calculated units equivalent (123)
    // encoder.positionUnits(123);

    // encoder.reset(); // -- Reset last read, set ticks to 0

    Serial.printf("ticks: %d\tunits: %d\n", position_ticks, position_units);

    delay(100);
}
