// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Sequence.hpp"
#include "kf/mixin/Equatable.hpp"
#include "kf/primitives.hpp"

namespace kf {

struct ArrayTag {};

/// @brief Array. Owns items
/// @tparam T Item type
/// @tparam N Items total
template<typename T, usize N> struct Array :

    ArrayTag,
    Sequence<Array<T, N>, T>,
    mixin::Equatable<Array<T, N>>

{
    constexpr static auto length{N};

    T items[N];

    constexpr void fill(const T &value) noexcept {
        for (auto &item: *this) {
            item = value;
        }
    }

private:
    using Self = Array<T, N>;

    KF_IMPL_SEQUENCE(Self, T);

    constexpr T *getDataImpl() noexcept {
        return items;
    }

    constexpr usize lengthImpl() const noexcept {
        return N;
    }

    KF_IMPL_EQUATABLE(Self);
    constexpr bool IsEqualsImpl(const Self &other) const noexcept {
        for (auto i = 0u; i < N; i += 1) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }

        return true;
    }
};

}// namespace kf