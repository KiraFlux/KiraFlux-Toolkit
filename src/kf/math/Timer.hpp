// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::internal {

struct TimerConfig final {
    math::Milliseconds period;
};

}// namespace kf::internal

namespace kf::math {

struct Timer final : mixin::NonCopyable, mixin::Configurable<internal::TimerConfig> {

    using Config = internal::TimerConfig;

    using ::kf::mixin::Configurable<Config>::Configurable;

    /// @brief Start (restart) timer
    void start(Milliseconds now) noexcept {
        _last = now;
    }

    /// @brief Expired check
    /// @note Do not automatically reset
    [[nodiscard]] bool expired(Milliseconds now) noexcept {
        return elapsed(now) >= this->config().period;
    }

    /// @brief Get Time since last start()
    [[nodiscard]] Milliseconds elapsed(Milliseconds now) noexcept {
        return now - _last;
    }

    /// @brief Get Time before Timer expire
    [[nodiscard]] Milliseconds remaining(Milliseconds now) noexcept {
        const auto e = elapsed(now);
        if (e < this->config().period) {// because of unsigned arithmetic
            return this->config().period - e;
        } else {
            return 0;
        }
    }

private:
    Milliseconds _last{0};
};

}// namespace kf::math