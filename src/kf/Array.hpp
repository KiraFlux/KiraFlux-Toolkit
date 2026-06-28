// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <initializer_list>

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
    Sequence<Array<T>, T>,
    mixin::Equatable<Array<T>>

{
    constexpr Array() noexcept : items{} {}

    constexpr Array(std::initializer_list<T> list) noexcept : items{} {
        for (auto i = 0u; i < N; i += 1) {
            items[i] = list.begin()[i];
        }
    }

    constexpr void fill(const T &value) noexcept {
        for (auto &item: *this) {
            item = value;
        }
    }

private:
    T _items[N];
    using This = Array<T, N>;

    KF_IMPL_SEQUENCE(This, T);

    constexpr T *getDataImpl() noexcept {
        return _items;
    }

    constexpr usize lengthImpl() const noexcept {
        return N;
    }

    KF_IMPL_EQUATABLE(This);
    constexpr bool IsEqualsImpl(const This &other) const noexcept {
        for (auto i = 0u; i < N; i += 1) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }

        return true;
    }
};

}// namespace kf