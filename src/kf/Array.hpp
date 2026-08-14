// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    Array.hpp

#pragma once

#include "kf/Sequence.hpp"
#include "kf/core.hpp"

#include "kf/mixin/Equatable.hpp"

namespace kf {

struct ArrayTag {};

/// @brief    Fixed‑size owning array with sequence interface and fill/equality operations.
/// @tparam T Item type
/// @tparam N Items total
template<typename T, usize N> struct Array :

    ArrayTag,
    Sequence<Array<T, N>, T>,
    mixin::Equatable<Array<T, N>>

{
    using Self = Array<T, N>;

    constexpr static auto items_total{N};

    T items[N];

    constexpr void fill(T const &value) noexcept {
        for (auto &item: *this) {
            item = value;
        }
    }

private:
    KF_IMPL_SEQUENCE(Self, T);

    constexpr T *getDataImpl() noexcept {
        return items;
    }

    constexpr usize lengthImpl() const noexcept {
        return N;
    }

    KF_IMPL_EQUATABLE(Self);
    constexpr bool isEqualsImpl(Self const &other) const noexcept {
        for (auto i = 0u; i < N; i += 1) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }

        return true;
    }
};

}// namespace kf