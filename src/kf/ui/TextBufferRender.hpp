// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <math.h> // NOLINT(*-deprecated-headers) // for AVR capability

#include "kf/Function.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/memory/ArrayString.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/ui/Render.hpp"


namespace kf {// NOLINT(*-concat-nested-namespaces) // for c++11 capability
namespace ui {

/// @brief Text-based UI rendering system for terminal/console output
/// @tparam N Text buffer capacity in characters
/// @note Implements Render CRTP interface for character-based display
template<usize N> struct TextBufferRender : Render<TextBufferRender<N>> {
    friend struct Render<TextBufferRender<N>>;

    using Glyph = u8; ///< Text interface measurement unit in glyphs

    /// @brief Text renderer configuration settings
    struct Config {
        Function<void(StringView)> on_render_finish{nullptr}; ///< Callback invoked when rendering completes

        Glyph row_max_length{16};       ///< Maximum characters per row
        Glyph rows_total{4};            ///< Total available rows in display
        Glyph float_places{2};          ///< Decimal places for float
        Glyph double_places{4};         ///< Decimal places for double
        bool title_centered{true};      ///< Render Title centered

        Config(const Config &) = delete;
    };

    Config config{};          ///< Current renderer configuration
    ArrayString<N> buffer{};  ///< Output buffer for rendered text

private:
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
        kf_nodiscard bool canWrite(Glyph row_max_length) const {
            return col < row_max_length;
        }

        /// @brief Advance cursor position by N characters
        void advance(Glyph count, Glyph row_max_length) {
            col += count;
            if (col >= row_max_length) {
                newline();
            }
        }
    } cursor;

    /// @brief Helper to write character with cursor tracking
    /// @param ch Character to write
    void writeChar(char ch) {
        if (buffer.full()) { return; }
        if (cursor.row >= config.rows_total) { return; }

        switch (ch) {
            case '\n':cursor.newline();
                break;

            case '\x82': // Start contrast
                cursor.contrast = true;
                break;

            case '\x80': // End contrast
                cursor.contrast = false;
                break;

            default:
                if (not cursor.canWrite(config.row_max_length)) {
                    // If row is full, and we're in contrast mode, exit it
                    if (cursor.contrast) {
                        (void) buffer.push('\x80');
                        cursor.contrast = false;
                    }
                    return;
                }
                cursor.advance(1, config.row_max_length);
                break;
        }

        (void) buffer.push(ch);
    }

    /// @brief Write string with cursor tracking
    void writeString(StringView str) {
        for (char ch: str) {
            writeChar(ch);
        }
    }

    void writeReal(f64 real, u8 rounding) {
        ArrayString<24> temp; // Enough for double with precision
        (void) temp.append(real, rounding);
        writeString(temp.view());
    }


    // Render Interface Implementation

    kf_nodiscard usize widgetsAvailableImpl() const {
        // Subtract 1 for title row
        if (config.rows_total > cursor.row + 1) {
            return config.rows_total - cursor.row - 1;
        } else {
            return 0;
        }
    }

    void prepareImpl() {
        buffer.clear();
        cursor.reset();
    }

    void finishImpl() {
        if (config.on_render_finish) {
            config.on_render_finish(buffer.view());
        }
    }

    void titleImpl(StringView title) {
        writeChar('\xF0');
        writeChar('\xBA');
        if (config.title_centered) {
            const auto spaces = kf::max(0, (int(config.row_max_length) - int(title.size())) / 2);
            for (int i = 0; i < spaces; i += 1) {
                writeChar(' ');
            }
        }
        writeString(title);
        writeChar('\n');
        writeChar('\x80');
    }

    void checkboxImpl(bool enabled) {
        constexpr StringView on{"==\xB2[ 1 ]\x80"};
        constexpr StringView off{"\xB1[ 0 ]\x80--"};
        writeString(enabled ? on : off);
    }

    // Value rendering implementations
    void valueImpl(StringView str) { writeString(str); }

    void valueImpl(bool value) {
        constexpr StringView _true{"\xF2true\x80"};
        constexpr StringView _false{"\xF1""false\x80"};
        writeString(value ? _true : _false);
    }

    void valueImpl(i32 integer) {
        ArrayString<12> temp; // Enough for 32-bit int
        (void) temp.append(integer);
        writeString(temp.view());
    }

    void valueImpl(f32 real) {
        writeReal(static_cast<f64>(real), config.float_places);
    }

    void valueImpl(f64 real) {
        writeReal(real, config.double_places);
    }

    // Decoration rendering

    void arrowImpl() { writeString("\xF6-> "); }

    void colonImpl() { writeString("\xF6: \x80"); }

    void beginFocusedImpl() { writeChar('\x81'); }

    void endFocusedImpl() { writeChar('\x80'); }

    void beginBlockImpl() {
        writeChar('\xF5');
        writeChar('[');
    }

    void endBlockImpl() {
        writeChar(']');
        writeChar('\x80');
    }

    void beginAltBlockImpl() {
        writeChar('\xF5');
        writeChar('<');
    }

    void endAltBlockImpl() {
        writeChar('>');
        writeChar('\x80');
    }

    void beginWidgetImpl(usize) {} // No-op for text renderer
    void endWidgetImpl() { writeChar('\n'); }
};

} // namespace ui
} // namespace kf