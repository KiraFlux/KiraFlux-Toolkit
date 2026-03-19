// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct QuitableTag {};

/// @brief CRTP mixin quit.
/// @tparam Impl Derived class (must provide `quitImpl()`).
template<typename Impl> struct Quitable : QuitableTag {

    /// @brief Quit: release resources, terminate, etc.
    void quit() noexcept { static_cast<Impl *>(this)->quitImpl(); }
};

}// namespace kf::mixin

#define KF_IMPL_QUITABLE(__impl__) friend struct kf::mixin::Quitable<__impl__>