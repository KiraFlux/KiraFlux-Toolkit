// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"

namespace kf {// NOLINT(*-concat-nested-namespaces) // for c++11 capability
namespace ui {

/// @brief Incoming UI event with type and value packed into single byte
/// @note Uses 3 bits for type and 5 bits for value (with sign extension)
struct Event {

private:
    using Storage = u8;

    static constexpr unsigned event_bits_total = sizeof(Storage) * 8;
    static constexpr Storage event_value_full = (1 << event_bits_total) - 1;

    static constexpr unsigned type_bits = 3;
    static constexpr unsigned value_bits = event_bits_total - type_bits;
    static constexpr unsigned sign_bit_mask = 1 << (value_bits - 1);

    static constexpr Storage value_mask = (1 << value_bits) - 1;
    static constexpr Storage type_mask = event_value_full & ~value_mask;

    Storage storage;///< Packed event data (type + value)

public:
    /// @brief Event type identifiers
    /// @note Uses 3 bits, allowing up to 8 distinct event types
    enum class Type : Storage {
        None = (0 << value_bits),             ///< No event (placeholder)
        Update = (1 << value_bits),           ///< Forced render request
        PageCursorMove = (2 << value_bits),   ///< Page cursor movement (may contain value)
        WidgetClick = (3 << value_bits),      ///< Widget click/tap
        WidgetValueChange = (4 << value_bits),///< Widget value change (may contain value)
    };

    using Value = i8;///< Event value primitive (5-bit signed)

    /// @brief Construct event with type and optional value
    /// @param type Event type
    /// @param value Event value (default: 0)
    constexpr explicit Event(Type type, Value value = 0) :
        storage{
            static_cast<Storage>(
                (static_cast<Storage>(type) & type_mask) |
                (static_cast<Storage>(value) & value_mask))} {}

    /// @brief Get event type
    /// @return Event type enum value
    kf_nodiscard constexpr Type type() const {
        return static_cast<Type>(storage & type_mask);
    }

    /// @brief Get event value with sign extension
    /// @return Signed value (-16 to +15 range for 5-bit storage)
    kf_nodiscard Value value() const {
        const auto result = static_cast<Value>(storage & value_mask);
        return (result & sign_bit_mask) ? static_cast<Value>(result | ~value_mask) : result;
    }

    // Predefined event instances

    /// @brief Create None event
    /// @return None event instance
    static constexpr Event None() { return Event{Type::None}; }

    /// @brief Create Update event (forces redraw)
    /// @return Update event instance
    static constexpr Event Update() { return Event{Type::Update}; }

    /// @brief Create PageCursorMove event with offset
    /// @param offset Cursor movement offset
    /// @return PageCursorMove event instance
    static constexpr Event PageCursorMove(Value offset) { return Event{Type::PageCursorMove, offset}; }

    /// @brief Create WidgetClick event
    /// @return WidgetClick event instance
    static constexpr Event WidgetClick() { return Event{Type::WidgetClick}; }

    /// @brief Create WidgetValueChange event with delta
    /// @param delta Value change amount
    /// @return WidgetValueChange event instance
    static constexpr Event WidgetValueChange(Value delta) { return Event{Type::WidgetValueChange, delta}; }
};

}// namespace ui
}// namespace kf