// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Flush.hpp
/// @brief   Adds flush() to force output of buffered data.

#pragma once

namespace kf::mixin {

struct FlushTag {};

/// @brief Adds `flush()` to force transmission of buffered data.
/// @tparam Impl Flush implementation class with `void flushImpl() noexcept` method
template<typename Impl> struct Flush : FlushTag {

    /// @brief Flush
    void flush() noexcept {
        static_cast<Impl *>(this)->flushImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_FLUSH(...) friend struct ::kf::mixin::Flush<__VA_ARGS__>
