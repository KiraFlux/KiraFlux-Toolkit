// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/tuner/Tag.hpp"

namespace kf::tuner {

template<typename Impl> struct Tuner : tuner::Tag {

    [[nodiscard]] bool running() const noexcept { return impl().runningImpl(); }

    void start() noexcept { impl().startImpl(); }

    void poll() noexcept { impl().pollImpl(); }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::tuner