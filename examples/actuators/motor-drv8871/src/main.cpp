// KiraFlux-Toolkit Demo 'actuators: motor powered by DRV8871'
#include <Arduino.h>

#include <kf/drivers/actuators/DRV8871.hpp>
#include <kf/gpio/ArduinoGPIO.hpp>

using PwmOutput = kf::gpio::ArduinoGPIO::PwmOutput;
using DRV8871 = kf::drivers::actuators::DRV8871<PwmOutput>;

// static/global: must outlive motor
PwmOutput::Config forward_pwm_config{
    .frequency_hz = 30'000,// typical for DC motors
    .resolution_bits = 10,
    .pin = static_cast<kf::u8>(GPIO_NUM_26),
    .channel = 0,
};

PwmOutput::Config backward_pwm_config{
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
    PwmOutput{/* & */ forward_pwm_config},
    PwmOutput{/* & */ backward_pwm_config},
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
