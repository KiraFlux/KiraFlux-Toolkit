// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::meta {

/// @brief CRTP base class providing static downcasting to derived type.
/// @tparam Impl The derived class type.
/// @note Use this to implement static polymorphism without virtual calls.
template<typename Impl> struct CRTP {
    /// @return Reference to derived object.
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }

    /// @return Const reference to derived object.
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::meta

#define KF_IMPL(__base__) friend struct __base__