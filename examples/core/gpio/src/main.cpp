// Demo: kf::gpio basic usage

#include <Arduino.h>
#include <kf/arduino/gpio.hpp>

using kf::arduino::ArduinoAdcInput;
using kf::arduino::ArduinoDigitalInput;
using kf::arduino::ArduinoDigitalOutput;
using kf::arduino::ArduinoPwmOutput;

ArduinoDigitalInput button{GPIO_NUM_10, ArduinoDigitalInput::Pull::ExternalDown};// button to GND
ArduinoDigitalOutput led{GPIO_NUM_12};                                           // LED with resistor
ArduinoAdcInput adc{GPIO_NUM_34};                                                // potentiometer (12‑bit)

ArduinoPwmOutput::Config ledc_config{
    .frequency_hz = 10000,
    .resolution_bits = 10,
    .gpio_num = static_cast<kf::u8>(GPIO_NUM_13),
};

ArduinoPwmOutput pwm{ledc_config};

void setup() {
    Serial.begin(115200);

    button.init();
    led.init();
    adc.init();
    (void) pwm.init();

    ArduinoAdcInput::resolution(12);
    Serial.println(ArduinoAdcInput::resolution());// get current adc resolution bits
    Serial.println(ArduinoAdcInput::maxValue());  // get current max value with current resolution
}

void loop() {
    static auto pwm_max = pwm.maxDuty();

    // Button -> LED
    led.write(button.read());

    // Print ADC value (0..4095)
    Serial.println(adc.read());

    // PWM sawtooth (0 -> 1023 -> 0...)
    static int i = 0;
    pwm.write(i);
    i += 1;
    i &= pwm_max;

    delay(10);
}