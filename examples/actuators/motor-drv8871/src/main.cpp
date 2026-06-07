// KiraFlux-Toolkit Demo 'actuators: motor powered by DRV8871'
#include <Arduino.h>

#include <kf/drivers/actuators/DRV8871.hpp>
#include <kf/gpio/ArduinoGPIO.hpp>

using kf::gpio::ArduinoGPIO;
using DRV8871 = kf::drivers::actuators::DRV8871<ArduinoGPIO>;

// static/global: must outlive motor
ArduinoGPIO::PwmOutput::Config forward_pwm_config{
    .frequency_hz = 30'000,// typical for DC motors
    .resolution_bits = 10,
    .pin = static_cast<kf::u8>(GPIO_NUM_26),
    .channel = 0,
};

ArduinoGPIO::PwmOutput::Config backward_pwm_config{
    .frequency_hz = 30'000,// typical for DC motors
    .resolution_bits = 10,
    .pin = static_cast<kf::u8>(GPIO_NUM_25),
    .channel = 0,
};

// static/global: must outlive motor
DRV8871::Config driver_config{
    .max_input = 1000,        // max control value
    .forward_dead_zone = 500, // pwm
    .backward_dead_zone = 500,// pwm
};

DRV8871 motor{
    /* & */ driver_config,
    ArduinoGPIO::PwmOutput{/* & */ forward_pwm_config},
    ArduinoGPIO::PwmOutput{/* & */ backward_pwm_config},
};

void setup() {
    Serial.begin(115200);

    if (not motor.init()) {
        Serial.println("motor init fail!");
        return;
    }

    delay(1000);

    motor.set(+1000);
    Serial.println("CW");
    delay(2000);

    motor.set(-500);
    Serial.println("CCW");
    delay(2000);

    motor.stop();
    Serial.println("Done");
}

void loop() {}
