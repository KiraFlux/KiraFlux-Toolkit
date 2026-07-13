// KiraFlux-Toolkit Demo 'sensor-sharp'
#include <Arduino.h>

#include <kf/arduino/ArduinoGPIO.hpp>
#include <kf/drivers/sensors/Sharp.hpp>

using AdcInput = kf::arduino::ArduinoGPIO::AdcInput;
using Sharp = kf::drivers::sensors::Sharp<AdcInput>;

Sharp my_sensor{AdcInput{GPIO_NUM_30}};

void setup() {
    Serial.begin(115200);

    AdcInput::resolution(10);
    my_sensor.init();
}

void loop() {
    const kf::math::Millimeters distance = my_sensor.read();

    Serial.println(distance);

    delay(50);
}
