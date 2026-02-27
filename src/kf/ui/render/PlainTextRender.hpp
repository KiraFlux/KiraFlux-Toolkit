// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/ArrayString.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui {

/// @brief Text-based UI rendering system for terminal/console output
/// @tparam N Text buffer capacity in characters
/// @note Implements Render CRTP interface for character-based display
template<usize N> struct PlainTextRender : Render<PlainTextRender<N>> {
    friend struct Render<PlainTextRender<N>>;

    using Glyph = u8;///< Text interface measurement unit in glyphs

    /// @brief Text renderer configuration settings
    struct Config {
        Function<void(StringView)> on_render_finish{nullptr};///< Callback invoked when rendering completes

        Glyph row_max_length{16}; ///< Maximum characters per row
        Glyph rows_total{4};      ///< Total available rows in display
        Glyph float_places{2};    ///< Decimal places for float
        Glyph double_places{4};   ///< Decimal places for double
        bool title_centered{true};///< Render Title centered

        Config(const Config &) = delete;
    };

private:
    ArrayString<N> buffer{};///< Output buffer for rendered text
    Config config{};        ///< Current renderer configuration

    /// @brief Cursor state for tracking rendering position
    struct Cursor {
        Glyph row{0};        ///< Current row position
        Glyph col{0};        ///< Current column position
        bool contrast{false};///< Whether we're in contrast mode

        /// @brief Reset cursor to beginning
        void reset() { *this = {}; }

        /// @brief Move to next line
        void newline() {
            row += 1;
            col = 0;
        }

        /// @brief Check if we can write more characters in current row
        /// @param row_max_length Maximum columns per row
        [[nodiscard]] bool canWrite(Glyph row_max_length) const noexcept {
            return col < row_max_length;
        }

        /// @brief Advance cursor position by N characters
        void advance(Glyph count, Glyph row_max_length) noexcept {
            col += count;
            if (col >= row_max_length) {
                newline();
            }
        }
    } cursor;

public:
    Config &getConfig() { return config; }

    /// @brief Helper to write character with cursor tracking
    /// @param ch Character to write
    void writeChar(char ch) noexcept {
        if (buffer.full() or cursor.row >= config.rows_total) { return; }

        if (ch == '\n') {
            cursor.newline();
            (void) buffer.push(ch);
            return;
        }

        if (not cursor.canWrite(config.row_max_length)) { return; }

        cursor.advance(1, config.row_max_length);
        (void) buffer.push(ch);
    }

    /// @brief Write string with cursor tracking
    void writeString(StringView str) noexcept {
        for (char ch: str) {
            writeChar(ch);
        }
    }

    void writeReal(f64 real, u8 rounding) noexcept {
        ArrayString<24> temp;// Enough for double with precision
        (void) temp.append(real, rounding);
        writeString(temp.view());
    }

private:
    // Render Interface Implementation

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept {
        // Subtract 1 for title row
        if (config.rows_total > cursor.row + 1) {
            return config.rows_total - cursor.row - 1;
        } else {
            return 0;
        }
    }

    void prepareImpl() noexcept {
        buffer.clear();
        cursor.reset();
    }

    void finishImpl() noexcept {
        if (config.on_render_finish) {
            config.on_render_finish(buffer.view());
        }
    }

    void titleImpl(StringView title) noexcept {
        if (config.title_centered) {
            const auto spaces = kf::max(0, (int(config.row_max_length) - int(title.size())) / 2);
            for (int i = 0; i < spaces; i += 1) {
                writeChar(' ');
            }
        }
        writeString(title);
        writeChar('\n');
    }

    void checkboxImpl(bool enabled) noexcept {
        constexpr StringView on{"==( 1 )"};
        constexpr StringView off{"( 0 )--"};
        writeString(enabled ? on : off);
    }

    template<typename T> void sliderImpl(T value, T min_value, T max_value, bool show_numeric_value) noexcept {
        writeString("SLIDER:");
        if (show_numeric_value) {
            value(value);
        }
    }

    // Value rendering implementations
    void valueImpl(StringView str) noexcept { writeString(str); }

    void valueImpl(bool value) noexcept {
        constexpr StringView _true{"true"};
        constexpr StringView _false{"false"};
        writeString(value ? _true : _false);
    }

    void valueImpl(i32 integer) noexcept {
        ArrayString<12> temp;// Enough for 32-bit int
        (void) temp.append(integer);
        writeString(temp.view());
    }

    void valueImpl(f32 real) noexcept {
        writeReal(static_cast<f64>(real), config.float_places);
    }

    void valueImpl(f64 real) noexcept {
        writeReal(real, config.double_places);
    }

    // Decoration rendering

    void arrowImpl() noexcept { writeString("-> "); }

    void colonImpl() noexcept { writeString(": "); }

    void beginFocusedImpl() noexcept { writeString("* "); }

    void endFocusedImpl() noexcept {}

    void beginBlockImpl() noexcept { writeChar('['); }

    void endBlockImpl() noexcept { writeChar(']'); }

    void beginAltBlockImpl() noexcept { writeChar('<'); }

    void endAltBlockImpl() noexcept { writeChar('>'); }

    void beginWidgetImpl(usize) noexcept {}

    void endWidgetImpl() noexcept { writeChar('\n'); }
};

}// namespace kf::ui