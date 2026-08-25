// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    driver/Driver.hpp
/// @brief   CRTP base for drivers.

#pragma once

#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::driver {

struct DriverTag {};

/// @brief Driver CRTP interface
/// @tparam Impl Driver implementation class
/// @tparam InitResult Return type of driver's `init()`
template<typename Impl, typename InitResult> struct Driver :

    DriverTag,
    mixin::NonCopyable,
    mixin::Initable<Impl, InitResult()>

{};

}// namespace kf::driver

#define KF_IMPL_DRIVER(__impl__, ...) KF_IMPL_INITABLE(__impl__, __VA_ARGS__())
