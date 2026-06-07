// KiraFlux-Toolkit Demo 'actuators-motor-l293n'
#include <Arduino.h>

#include <kf/drivers/actuators/Motor.hpp>
#include <kf/gpio/ArduinoGPIO.hpp>

using kf::gpio::ArduinoGPIO;

using L293N = kf::drivers::actuators::L298nMotor<ArduinoGPIO::PwmOutput, ArduinoGPIO::DigitalOutput>;

// static/global: must outlive motor
ArduinoGPIO::PwmOutput::Config pwm_config{
    .frequency_hz = 30'000,// typical for DC motors
    .resolution_bits = 10,
    .pin = static_cast<kf::u8>(GPIO_NUM_26),
    .channel = 0,
};

// static/global: must outlive motor
L293N::Config driver_config{
    .dead_zone = 500,// motor start moving with pwm >= 500
    .max_duty = pwm_config.maxDuty(),
    .normal_direction = L293N::Config::Direction::CW,// motor.write(1.0) -> Clock wise rotation
};

L293N motor{
    /* & */ driver_config,
    ArduinoGPIO::PwmOutput{/* & */ pwm_config},
    ArduinoGPIO::DigitalOutput{GPIO_NUM_25},
};

void setup() {
    Serial.begin(115200);

    if (not motor.init()) {
        Serial.println("motor init fail!");
        return;
    }

    delay(1000);

    motor.write(+1.0f);
    Serial.println("CW");
    delay(2000);

    motor.write(-1.0f);
    Serial.println("CCW");
    delay(2000);

    motor.disable();
    Serial.println("Done");
}

void loop() {}
