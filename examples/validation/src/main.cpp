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

    // Constructor to allow easy initialization.
    MotorConfig(int speed, float limit, bool en) noexcept :
        max_speed{speed}, current_limit{limit}, enabled{en} {}

    // This method is called by `check()`.
    // It receives a Validator object that collects the results.
    void checkImpl(kf::Validator &v) const noexcept {
        // The macro logs the condition string and the result (info on success, error on failure)
        // and increments the error counter if the condition is false.
        kf_Validator_check(v, logger, max_speed > 0);
        kf_Validator_check(v, logger, current_limit >= 0.1f and current_limit <= 5.0f);
    }
};

void setup() {
    Serial.begin(115200);

    // Redirect all logger output to Serial.
    kf::Logger::writer = [](kf::memory::StringView s) {
        Serial.write(s.data(), s.size());
    };

    // Create two configuration objects: one valid, one invalid.
    MotorConfig valid{100, 2.5f, true};
    MotorConfig invalid{0, -99.0f, true};

    // The `check()` method runs all checks and returns true if no errors occurred.
    // All validation messages are automatically printed via the logger.
    Serial.print("valid config: ");
    Serial.println(valid.check() ? "PASS" : "FAIL");

    Serial.print("invalid config: ");
    Serial.println(invalid.check() ? "PASS" : "FAIL");
}

void loop() {}