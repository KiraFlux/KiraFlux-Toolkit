// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/core.hpp>// For kf::usize

namespace kf::mixin {

struct ExtraAllocationLengthTag {};

template<typename Impl> struct ExtraAllocationLength : ExtraAllocationLengthTag {

    [[nodiscard]] static constexpr kf::usize extraAllocationLength(auto const &...args) noexcept {
        return Impl::getExtraAllocationLengthImpl(args...);
    }
};

}// namespace kf::mixin

#define KF_IMPL_EXTRA_ALLOCATION_LENGTH(...) friend struct ::kf::mixin::ExtraAllocationLength<__VA_ARGS__>