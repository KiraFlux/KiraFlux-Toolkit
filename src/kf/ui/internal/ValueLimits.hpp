// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

template<typename T> struct value_limit_min;

template<> struct value_limit_min<int> {
    static constexpr int value{0};
};

template<> struct value_limit_min<float> {
    static constexpr float value{0.0f};
};

template<> struct value_limit_min<double> {
    static constexpr double value{0.0};
};


template<typename T> struct value_limit_max;

template<> struct value_limit_max<int> {
    static constexpr int value{100};
};

template<> struct value_limit_max<float> {
    static constexpr float value{1.0f};
};

template<> struct value_limit_max<double> {
    static constexpr double value{1.0};
};

template<typename T> struct ValueLimits {
    static constexpr T value_min{value_limit_min<T>::value};
    static constexpr T value_max{value_limit_max<T>::value};
};