// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"
#include "kf/StringView.hpp"

namespace kf {

/// @brief App Initial Context
struct Init {

    /// @brief application logger
    Logger logger;
};

/// @brief KiraFlux Toolkit application entry point
void main(Init &init);

}// namespace kf

#define KF_NOTICE_STRING "KiraFlux Toolkit v0.3.5"

#ifdef ARDUINO

#include <Arduino.h>

#ifndef KF_SERIAL_BAUDRATE
#define KF_SERIAL_BAUDRATE 115200
#endif

void setup() {

    kf::Init init{
        .logger = kf::Logger{"main"},
    };

    kf::Logger::writer = [](kf::StringView str) {
        if (not str.empty()) {
            Serial.write(str.data(), str.length());
        }
    };

    Serial.begin(KF_SERIAL_BAUDRATE);

    init.logger.info(KF_NOTICE_STRING);

    kf::main(init);
}

void loop() {}

#else

#include <iostream>

int main() {

    kf::Init init{
        .logger = kf::Logger{"main"},
    };

    kf::Logger::writer = [](kf::StringView str) {
        if (not str.empty()) {
            std::cout.write(str.data(), str.length());
        }
    };

    init.logger.info(KF_NOTICE_STRING);

    kf::main(init);

    return 0;
}

#endif

#undef KF_NOTICE_STRING
