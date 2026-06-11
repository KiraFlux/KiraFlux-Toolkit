// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/meta/BitTraits.hpp"
#include "kf/primitives.hpp"

namespace kf::ui {

struct EventTag {};

/// @brief Incoming UI event with kind and value packed into single byte
/// @tparam N Number of bits for event value
template<usize N> struct Event : EventTag {
    static constexpr auto type_bits{2u};
    static constexpr auto value_bits{N};
    static constexpr auto total_bits{static_cast<usize>(type_bits + value_bits)};

    using Value = typename meta::BitTraits<value_bits>::min_signed;
    using Storage = typename meta::BitTraits<total_bits>::min_unsigned;
    using UnsignedValue = typename meta::BitTraits<value_bits>::min_unsigned;

    static constexpr auto storage_bits{static_cast<usize>(sizeof(Storage) * 8)};
    static constexpr auto value_max{static_cast<Value>((UnsignedValue{1u} << (value_bits - 1)) - 1)};
    static constexpr auto value_min{static_cast<Value>(-value_max - 1)};
    static constexpr auto event_value_full{static_cast<Storage>(~Storage{0})};
    static constexpr auto value_mask{static_cast<Storage>((Storage{1} << value_bits) - 1)};
    static constexpr auto type_mask{static_cast<Storage>(event_value_full & (~value_mask))};
    static constexpr auto sign_bit_mask{static_cast<Storage>(Storage{1} << (value_bits - 1))};

    enum class Kind : Storage {
        Update = static_cast<Storage>(Storage{0} << value_bits),
        PageCursorMove = static_cast<Storage>(Storage{1} << value_bits),
        WidgetClick = static_cast<Storage>(Storage{2} << value_bits),
        WidgetValue = static_cast<Storage>(Storage{3} << value_bits),
    };

    explicit constexpr Event(Kind kind, Value value = 0) noexcept :
        _storage{static_cast<Storage>((static_cast<Storage>(kind) & type_mask) | static_cast<Storage>((clamp(value, value_min, value_max)) & value_mask))} {}

    [[nodiscard]] constexpr Kind kind() const noexcept {
        return static_cast<Kind>(_storage & type_mask);
    }

    [[nodiscard]] Value value() const noexcept {
        const auto raw = static_cast<Value>(_storage & value_mask);
        return (raw & sign_bit_mask) ? static_cast<Value>(raw | ~value_mask) : raw;
    }

    [[nodiscard]] static constexpr Event update() noexcept {
        return Event{Kind::Update};
    }

    [[nodiscard]] static constexpr Event pageCursorMove(Value offset) noexcept {
        return Event{Kind::PageCursorMove, offset};
    }

    [[nodiscard]] static constexpr Event widgetClick() noexcept {
        return Event{Kind::WidgetClick};
    }

    [[nodiscard]] static constexpr Event widgetValue(Value value) noexcept {
        return Event{Kind::WidgetValue, value};
    }

private:
    Storage _storage;
};

}// namespace kf::ui