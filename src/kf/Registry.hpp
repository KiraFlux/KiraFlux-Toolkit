// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    Registry.hpp

#pragma once

#include <utility>// For std::forward

#include "kf/Arena.hpp"
#include "kf/NoneType.hpp"
#include "kf/Option.hpp"
#include "kf/Slice.hpp"
#include "kf/Stack.hpp"
#include "kf/core.hpp"

#include "kf/mixin/Match.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

template<typename T> struct Registry;

namespace internal {

template<typename T> struct RegistryBase : mixin::Resettable<RegistryBase<T>> {

    explicit constexpr RegistryBase(Arena &arena, usize count) noexcept :
        _items{arena.allocate<T *>(count)} {}

    [[nodiscard]] constexpr auto items() noexcept {
        return _items.slice();
    }

    [[nodiscard]] constexpr auto items() const noexcept {
        return _items.slice();
    }

    template<typename... Args> [[nodiscard]] auto add(Arena &arena, Args &&...args) noexcept -> Option<T &> {
        if (_items.full()) { return none; }

        if (auto maybe_item = arena.create<T>(std::forward<Args>(args)...); maybe_item.isSome()) {
            (void) _items.write(&maybe_item.unwrap());
            return maybe_item;
        }

        return none;
    }

private:
    Stack<T *> _items;

    KF_IMPL_RESETTABLE(RegistryBase<T>);
    constexpr void resetImpl() noexcept {
        for (auto item: _items) {
            item->~T();
        }
        _items.reset();
    }
};

}// namespace internal

template<typename T> struct Registry : internal::RegistryBase<T> {

    using internal::RegistryBase<T>::RegistryBase;
};

template<implements<mixin::MatchTag> T> struct NamedRegistryBase : internal::RegistryBase<T> {

    using internal::RegistryBase<T>::RegistryBase;

    auto get(typename T::MatchingType value) noexcept -> Option<T &> {
        return this->items().firstWhere([value](auto item) { return item->match(value); });
    }

    auto get(typename T::MatchingType value) const noexcept -> Option<T const &> {
        return this->items().firstWhere([value](auto item) { return item->match(value); });
    }
};

}// namespace kf