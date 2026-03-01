#include <Arduino.h>
#include <kf/drivers/zms/PwmPositionServo.hpp>

using kf::PwmPositionServo;

// 50 Hz, 16-bit resolution works for most RC servos
static const PwmPositionServo::PwmConfig pwm_config{
    .ledc_frequency_hz = 50,
    .ledc_resolution_bits = 16,
};

// GPIO13, channel 0; 0..180 deg range
static const PwmPositionServo::DriverConfig driver_config{
    .signal_pin = GPIO_NUM_13,
    .ledc_channel = 0,
    .min_angle = 0,
    .max_angle = 180,
};

// Typical 500‑2500 us pulse width for 0‑180 deg
static const PwmPositionServo::PulseConfig pulse_config{
    .min_pulse = {500, 0},
    .max_pulse = {2500, 180},
};

static PwmPositionServo servo{pwm_config, driver_config, pulse_config};

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (not servo.init()) {
        Serial.println("Servo init failed");
        return;
    }

    // sweep 0 -> 180 -> 0
    for (int angle = -180; angle <= 180; angle += 1) {
        servo.set(abs(180 - angle));
        delay(20);
    }
}

void loop() {}