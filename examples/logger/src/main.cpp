#include <Arduino.h>

#include <kf/Logger.hpp>

constexpr auto my_logger = kf::Logger::create("Example");

void setup() {
    Serial.begin(115200);

    // Setup global write handler
    kf::Logger::writer = [](kf::StringView str) {
        Serial.write(str.data(), str.size());
    };

    // send with different levels
    my_logger.debug("test");
    my_logger.info("test");
    my_logger.warn("test");
    my_logger.error("test");
}

void loop() {}