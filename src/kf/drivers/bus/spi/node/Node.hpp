// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/bus/spi/node/Tag.hpp"

namespace kf::drivers::bus::spi::node {

template<typename Impl> struct Node : node::Tag {
    void init() noexcept { impl().initImpl(); }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::drivers::bus::spi::node
