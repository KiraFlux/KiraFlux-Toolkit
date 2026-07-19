// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"
#include "kf/StringView.hpp"

#define KF_NOTICE_STRING "KiraFlux Toolkit v0.3.5"

#ifndef KF_MAIN_LOGGER_KEY
#define KF_MAIN_LOGGER_KEY "main"
#endif

#ifndef KF_MAIN_LOGGER_BUFFER_LENGTH
#define KF_MAIN_LOGGER_BUFFER_LENGTH 256
#endif

namespace kf {

/// @brief App Initial Context
struct Init {

    /// @brief application logger
    Logger logger;
};

/// @brief KiraFlux Toolkit application entry point
void main(Init &init);

namespace internal {

void launch() noexcept {
    static char main_logger_buffer[(KF_MAIN_LOGGER_BUFFER_LENGTH)]{};

    Init init{
        .logger = Logger{
            (KF_MAIN_LOGGER_KEY),
            {main_logger_buffer},
        },
    };

    init.logger.info((KF_NOTICE_STRING));

    main(init);
}

}// namespace internal

}// namespace kf

#ifdef ARDUINO

#include <Arduino.h>

#ifndef KF_SERIAL_BAUDRATE
#define KF_SERIAL_BAUDRATE 115200
#endif

void setup() {
    Serial.begin(KF_SERIAL_BAUDRATE);

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

#undef KF_NOTICE_STRING
