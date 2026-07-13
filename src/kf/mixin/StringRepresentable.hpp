// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct StringRepresentableTag {};

/// @brief CRTP mixin that adds a `toString()` method
/// @tparam Impl       Implementation class (must implement `StringType toStringImpl() const noexcept`)
/// @tparam StringType Return type of `toString()`
template<typename Impl, typename StringType> struct StringRepresentable : StringRepresentableTag {

    /// @brief Get string representation of the object
    /// @return String of type `StringType`
    [[nodiscard]] constexpr StringType toString() const noexcept {
        return static_cast<const Impl *>(this)->toStringImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_STRING_REPRESENTABLE(__impl__, ...) friend struct ::kf::mixin::StringRepresentable<__impl__, __VA_ARGS__>
