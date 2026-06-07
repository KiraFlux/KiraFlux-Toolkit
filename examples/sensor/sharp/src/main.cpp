// KiraFlux-Toolkit Demo 'sensor-sharp'
#include <Arduino.h>

#include <kf/drivers/sensors/Sharp.hpp>
#include <kf/gpio/ArduinoGPIO.hpp>

using kf::gpio::ArduinoGPIO;
using Sharp = kf::drivers::sensors::Sharp<ArduinoGPIO>;

Sharp my_sensor{ArduinoGPIO::AdcInput{GPIO_NUM_30}};

void setup() {
    Serial.begin(115200);

    ArduinoGPIO::AdcInput::resolution(10);
    my_sensor.init();
}

void loop() {
    const kf::math::Millimeters distance = my_sensor.read();

    Serial.println(distance);

    delay(50);
}
