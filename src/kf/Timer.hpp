// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::internal {

struct TimerConfig final {

    /// @brief Timer value (Semantic depend on use - this value may represent: period, duration, etc..)
    math::Milliseconds value;
};

}// namespace kf::internal

namespace kf {

struct Timer final :

    mixin::NonCopyable,
    mixin::Configurable<internal::TimerConfig>

{
    using Config = internal::TimerConfig;

    using ::kf::mixin::Configurable<Config>::Configurable;

    /// @brief Start (restart) timer
    constexpr void start(math::Milliseconds now) noexcept {
        _last = now;
    }

    /// @brief Expired check
    /// @note Do not automatically reset
    [[nodiscard]] constexpr bool expired(math::Milliseconds now) noexcept {
        return elapsed(now) >= this->config().value;
    }

    /// @brief Get Time since last start()
    [[nodiscard]] constexpr math::Milliseconds elapsed(math::Milliseconds now) noexcept {
        return now - _last;
    }

    /// @brief Get Time before Timer expire
    [[nodiscard]] constexpr math::Milliseconds remaining(math::Milliseconds now) noexcept {
        const auto e = elapsed(now);
        if (e < this->config().value) {// because of unsigned arithmetic
            return this->config().value - e;
        } else {
            return 0;
        }
    }

private:
    math::Milliseconds _last{0};
};

}// namespace kf