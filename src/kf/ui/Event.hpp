// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/meta/bit_traits.hpp"

namespace kf {
namespace ui {

/// @brief Incoming UI event with type and value packed into single byte
/// @tparam V Number of bits for event value
template<u8 V> struct Event {
    static constexpr u8 type_bits = 2;
    static constexpr u8 value_bits = V;
    static constexpr u8 total_bits = type_bits + value_bits;

    using Value = typename meta::bit_traits<value_bits>::min_signed;
    using Storage = typename meta::bit_traits<total_bits>::min_unsigned;
    using UnsignedValue = typename meta::bit_traits<value_bits>::min_unsigned;

    static constexpr u8 storage_bits = sizeof(Storage) * 8;

    static constexpr Value value_max = static_cast<Value>((UnsignedValue(1) << (value_bits - 1)) - 1);
    static constexpr Value value_min = -value_max - 1;// безопасное вычисление

private:
    static constexpr Storage event_value_full = ~Storage(0);
    static constexpr Storage value_mask = (Storage(1) << value_bits) - 1;
    static constexpr Storage type_mask = event_value_full & ~value_mask;
    static constexpr Storage sign_bit_mask = Storage(1) << (value_bits - 1);

    Storage storage;

public:
    enum class Type : Storage {
        Update = Storage(0) << value_bits,
        PageCursorMove = Storage(1) << value_bits,
        WidgetClick = Storage(2) << value_bits,
        WidgetValueChange = Storage(3) << value_bits,
    };

    constexpr explicit Event(Type type, Value value = 0) noexcept
        : storage{
              static_cast<Storage>(
                  (static_cast<Storage>(type) & type_mask) |
                  (static_cast<Storage>(clamp(value, value_min, value_max)) & value_mask))} {}

    [[nodiscard]] constexpr Type type() const noexcept {
        return static_cast<Type>(storage & type_mask);
    }

    [[nodiscard]] Value value() const noexcept {
        auto raw = static_cast<Value>(storage & value_mask);
        return (raw & sign_bit_mask) ? static_cast<Value>(raw | ~value_mask) : raw;
    }

    [[nodiscard]] static constexpr Event update() noexcept { return Event{Type::Update}; }
    [[nodiscard]] static constexpr Event pageCursorMove(Value offset) noexcept {
        return Event{Type::PageCursorMove, offset};
    }
    [[nodiscard]] static constexpr Event widgetClick() noexcept { return Event{Type::WidgetClick}; }
    [[nodiscard]] static constexpr Event widgetValue(Value value) noexcept {
        return Event{Type::WidgetValueChange, value};
    }
};

}// namespace ui
}// namespace kf