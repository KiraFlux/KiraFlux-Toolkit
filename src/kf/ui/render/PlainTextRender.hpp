// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/ArrayString.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/ui/internal/ValuePlacement.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui::render {

/// @brief Text-based UI rendering system for terminal/console output
/// @tparam N Text buffer capacity in characters
/// @note Implements Render CRTP interface for character-based display
template<usize N> struct PlainTextRender : Render<PlainTextRender<N>> {

    using Glyph = u8;///< Text interface measurement unit in glyphs

    /// @brief Text renderer configuration
    struct Config {
        Function<void(memory::StringView)> on_render_finish{nullptr};///< Callback invoked when rendering completes

        Glyph row_max_length{16}; ///< Maximum characters per row
        Glyph rows_total{4};      ///< Total available rows in display
        Glyph float_places{2};    ///< Decimal places for float
        Glyph double_places{4};   ///< Decimal places for double
        bool title_centered{true};///< Render Title centered

        Config(const Config &) = delete;
    };

    constexpr explicit PlainTextRender(const Config &config) noexcept : _config{config} {}

private:
    memory::ArrayString<N> _buffer{};///< Output buffer for rendered text
    const Config &_config{};         ///< Current renderer configuration

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
    } _cursor{};

public:
    /// @brief Helper to write character with cursor tracking
    void writeChar(char ch) noexcept {
        if (_buffer.full() or _cursor.row >= _config.rows_total) { return; }

        if (ch == '\n') {
            _cursor.newline();
            (void) _buffer.push(ch);
            return;
        }

        if (not _cursor.canWrite(_config.row_max_length)) { return; }

        _cursor.advance(1, _config.row_max_length);
        (void) _buffer.push(ch);
    }

    /// @brief Write string with cursor tracking
    void writeString(memory::StringView str) noexcept {
        for (char ch: str) {
            writeChar(ch);
        }
    }

    void writeReal(f64 real, u8 rounding) noexcept {
        memory::ArrayString<24> temp;// Enough for double with precision
        (void) temp.append(real, rounding);
        writeString(temp.view());
    }

private:
    // Render Interface Implementation

    friend struct Render<PlainTextRender<N>>;

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept {
        // Subtract 1 for title row
        if (_config.rows_total > _cursor.row + 1) {
            return _config.rows_total - _cursor.row - 1;
        } else {
            return 0;
        }
    }

    void prepareImpl() noexcept {
        _buffer.clear();
        _cursor.reset();
    }

    void finishImpl() noexcept {
        if (_config.on_render_finish) {
            _config.on_render_finish(_buffer.view());
        }
    }

    void titleImpl(memory::StringView title) noexcept {
        if (_config.title_centered) {
            const auto spaces = kf::max(0, (int(_config.row_max_length) - int(title.size())) / 2);
            for (int i = 0; i < spaces; i += 1) {
                writeChar(' ');
            }
        }
        writeString(title);
        writeChar('\n');
    }

    void checkboxImpl(bool enabled) noexcept {
        constexpr memory::StringView on{"==( 1 )"};
        constexpr memory::StringView off{"( 0 )--"};
        writeString(enabled ? on : off);
    }

    template<typename T> void sliderImpl(
        T slider_value, T min_value, T max_value,
        internal::ValuePlacement value_placement) noexcept {

        // Textual now supports only show/hide placement
        if (internal::ValuePlacement::Hidden != value_placement) {
            this->value(slider_value);
            writeChar(' ');
        }

        writeChar('[');
        const usize start_col = _cursor.col;
        const usize inner_width = _config.row_max_length - start_col - 1;// -1 for closing char
        const usize fill = (slider_value - min_value) * inner_width / (max_value - min_value);

        for (usize i = 0; i < fill; i += 1) {
            writeChar('=');
        }

        writeChar('@');

        for (usize i = _cursor.col - start_col; i < inner_width; i += 1) {
            writeChar('-');
        }

        writeChar(']');
    }

    // Value rendering implementations
    void valueImpl(memory::StringView str) noexcept { writeString(str); }

    void valueImpl(bool slider_value) noexcept {
        constexpr memory::StringView label_true{"true"};
        constexpr memory::StringView label_false{"false"};
        writeString(slider_value ? label_true : label_false);
    }

    void valueImpl(i32 integer) noexcept {
        memory::ArrayString<12> temp;// Enough for 32-bit int
        (void) temp.append(integer);
        writeString(temp.view());
    }

    void valueImpl(f32 real) noexcept {
        writeReal(static_cast<f64>(real), _config.float_places);
    }

    void valueImpl(f64 real) noexcept {
        writeReal(real, _config.double_places);
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

}// namespace kf::ui::render