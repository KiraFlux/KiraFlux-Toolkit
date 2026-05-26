// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/StaticString.hpp"
#include "kf/primitives.hpp"

namespace kf::mixin {

struct StringRepresentableTag {};

template<typename Impl, kf::usize> struct StringRepresentable : StringRepresentableTag {
    static constexpr auto repr_string_capacity{N};

    [[nodiscard]] memory::StaticString<N> toString() const noexcept {
        return static_cast<const Impl *>(this)->toStringImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_STRING_REPRESENTABLE(__impl__, n) friend struct ::kf::mixin::StringRepresentable<__impl__, n>