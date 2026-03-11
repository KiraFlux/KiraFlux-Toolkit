// Demo: kf::gpio basic usage

#include <Arduino.h>
#include <kf/gpio/arduino.hpp>

using namespace kf::gpio::arduino;

DigitalInput button{GPIO_NUM_10, DigitalInput::Pull::External};// button to GND
DigitalOutput led{GPIO_NUM_12};                                // LED with resistor
AdcInput adc{GPIO_NUM_34};                                     // potentiometer (12‑bit)

PwmOutput::Config ledc_config{
    .frequency_hz = 10000,
    .resolution_bits = 10,
    .pin = static_cast<kf::u8>(GPIO_NUM_13),
    .channel = 0,
};

PwmOutput pwm{ledc_config};

void setup() {
    Serial.begin(115200);

    button.init();
    led.init();
    adc.init();
    (void) pwm.init();
}

void loop() {
    // Button → LED
    led.write(button.read());

    // Print ADC value (0-4095)
    Serial.println(adc.read());

    // PWM sawtooth (0 -> 1023 -> 0...)
    static int i = 0;
    pwm.write(i);
    i = (i + 1) & 1023;

    delay(10);
}