// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Indexable.hpp"
#include "kf/mixin/Length.hpp"
#include "kf/primitives.hpp"

namespace kf {

template<typename> struct Slice;// forward declaration

struct SequenceTag {};

/// @brief Sequence
/// @tparam Impl Sequence Implementation class with `lengthImpl` and `getDataImpl` methods
/// @tparam T Item type
template<typename Impl, typename T> struct Sequence :

    SequenceTag,
    mixin::Indexable<Sequence<Impl, T>, T>,
    mixin::Length<Impl, usize>

{
    using Self = Sequence<Impl, T>;

    [[nodiscard]] constexpr T *data() noexcept {
        return static_cast<Impl *>(this)->getDataImpl();
    }

    [[nodiscard]] constexpr const T *data() const noexcept {
        return const_cast<Sequence *>(this)->data();
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return this->length() == 0;
    }

    [[nodiscard]] constexpr T *begin() noexcept {
        return data();
    }

    [[nodiscard]] constexpr T *end() noexcept {
        return begin() + this->length();
    }

    [[nodiscard]] constexpr const T *begin() const noexcept {
        return data();
    }

    [[nodiscard]] constexpr const T *end() const noexcept {
        return begin() + this->length();
    }

    /// @brief Get mutable slice
    [[nodiscard]] constexpr Slice<T> slice() noexcept {
        return {
            this->data(),
            this->length(),
        };
    }

    /// @brief Get readonly slice
    [[nodiscard]] constexpr Slice<const T> slice() const noexcept {
        return {
            this->data(),
            this->length(),
        };
    }

private:
    KF_IMPL_INDEXABLE(Self, T);

    constexpr T &getItemImpl(usize index) noexcept {
        return data()[index];
    }
};

}// namespace kf

#define KF_IMPL_SEQUENCE(__impl__, ...)                  \
    friend struct ::kf::Sequence<__impl__, __VA_ARGS__>; \
    KF_IMPL_LENGTH(__impl__, ::kf::usize)
