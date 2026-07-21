// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"
#include "kf/StringView.hpp"

#ifndef KF_CONFIG_MAIN_LOGGER_KEY
#define KF_CONFIG_MAIN_LOGGER_KEY "main"
#endif

#ifndef KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH
#define KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH 256
#endif

#ifndef KF_CONFIG_SERIAL_BAUDRATE
#define KF_CONFIG_SERIAL_BAUDRATE 115200
#endif

namespace kf {

/// @brief App Context
struct Init {
    // TODO: add io - (platform-depended binary IO class instance)

    /// @brief application logger
    Logger logger;
};

/// @brief KiraFlux Toolkit application entry point
void main(Init &init);

namespace internal {

void launch() noexcept {
    static char main_logger_buffer[(KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH)]{};

    Init init{
        .logger = Logger{
            (KF_CONFIG_MAIN_LOGGER_KEY),
            {main_logger_buffer},
        },
    };

    // TODO: set logger writer here (use init.io)

    init.logger.info("KiraFlux Toolkit v0.3.5");

    main(init);
}

}// namespace internal

}// namespace kf

#ifdef ARDUINO

#include <Arduino.h>

void setup() {
    Serial.begin((KF_CONFIG_SERIAL_BAUDRATE));

    kf::Logger::writer = [](kf::StringView str) {
        if (not str.empty()) {
            Serial.write(str.data(), str.length());
            Serial.flush();
        }
    };

    kf::internal::launch();
}

void loop() {}

#else

#include <iostream>

int main() {

    kf::Logger::writer = [](kf::StringView str) {
        if (not str.empty()) {
            std::cout.write(str.data(), str.length());
            std::cout.flush();
        }
    };

    kf::internal::launch();

    return 0;
}

#endif
