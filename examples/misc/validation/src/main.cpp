#include <Arduino.h>
#include <kf/Logger.hpp>
#include <kf/validation.hpp>

// Example configuration structure that uses the validation framework.
// Inherits from kf::Validatable<MotorConfig> to gain the `check()` method.
struct MotorConfig : kf::Validatable<MotorConfig> {
    int max_speed;      // must be > 0
    float current_limit;// must be between 0.1 and 5.0
    bool enabled;       // any value allowed – no check needed

    // Each Validatable class should have its own logger for validation messages.
    static constexpr auto logger = kf::Logger::create("MotorCfg");

    // This method is called by `check()`.
    // It receives a Validator object that collects the results.
    void checkImpl(kf::Validator &v) const noexcept {
        // The macro logs the condition string and the result (info on success, error on failure)
        // and increments the error counter if the condition is false.
        KF_VALIDATOR_CHECK(v, logger, max_speed > 0);
        KF_VALIDATOR_CHECK(v, logger, current_limit >= 0.1f and current_limit <= 5.0f);
    }
};

void setup() {
    Serial.begin(115200);

    // Redirect all logger output to Serial.
    kf::Logger::writer = [](kf::memory::StringView s) {
        Serial.write(s.data(), s.size());
    };

    // Create two configuration objects: one valid, one invalid.
    MotorConfig valid{
        .max_speed = 100,
        .current_limit = 2.5f,
        .enabled = true,
    };

    MotorConfig invalid{
        .max_speed = 0,
        .current_limit = -99.0f,
        .enabled = true,
    };

    // All validation messages are automatically printed via the logger.
    kf::Validator validator{};

    Serial.print("valid config");
    valid.check(validator);

    Serial.print("invalid config");
    invalid.check(validator);
}

void loop() {}