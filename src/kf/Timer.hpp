// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Configured.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/units.hpp"

namespace kf::internal {

struct TimerConfig final {

    /// @brief Timer value (Semantic depend on use - this value may represent: period, duration, etc..)
    units::Milliseconds value;
};

}// namespace kf::internal

namespace kf {

struct Timer final :

    mixin::NonCopyable,
    mixin::Configured<internal::TimerConfig>

{
    using Config = internal::TimerConfig;

    using ::kf::mixin::Configured<Config>::Configured;

    /// @brief Start (restart) timer
    constexpr void start(units::Milliseconds now) noexcept {
        _last = now;
    }

    /// @brief Expired check
    /// @note Do not automatically reset
    [[nodiscard]] constexpr bool expired(units::Milliseconds now) noexcept {
        return elapsed(now) >= this->config().value;
    }

    /// @brief Get Time since last start()
    [[nodiscard]] constexpr units::Milliseconds elapsed(units::Milliseconds now) noexcept {
        return now - _last;
    }

    /// @brief Get Time before Timer expire
    [[nodiscard]] constexpr units::Milliseconds remaining(units::Milliseconds now) noexcept {
        const auto e = elapsed(now);
        if (e < this->config().value) {// because of unsigned arithmetic
            return this->config().value - e;
        } else {
            return 0;
        }
    }

private:
    units::Milliseconds _last{0};
};

}// namespace kf