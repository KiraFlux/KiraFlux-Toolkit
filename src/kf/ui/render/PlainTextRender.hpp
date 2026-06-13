// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/algorithm.hpp"
#include "kf/memory/StaticString.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Block.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/Placement.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::internal {

using Glyph = u8;///< Text interface measurement unit in glyphs

struct PlainTextRenderConfig final {
    Glyph row_max_length;///< Maximum characters per row
    Glyph rows_total;    ///< Total available rows in display
    Glyph float_places;  ///< Decimal places for float
    Glyph double_places; ///< Decimal places for double
    bool title_centered; ///< Render Title centered

    static constexpr auto defaults() noexcept {
        return PlainTextRenderConfig{
            .row_max_length = 16,
            .rows_total = 4,
            .float_places = 2,
            .double_places = 4,
            .title_centered = true,
        };
    }
};

/// @brief Cursor state for tracking rendering position
struct PlainTextRenderCursor {
    Glyph row{0}, col{0};///< Current position

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
};

}// namespace kf::internal

namespace kf::ui::render {

/// @brief Text-based UI rendering system for terminal/console output
/// @tparam N Text buffer capacity in characters
/// @note Implements Render CRTP interface for character-based display
template<usize N> struct PlainTextRender :

    Render<PlainTextRender<N>>,
    mixin::Callbacked<memory::StringView>,
    mixin::Configurable<internal::PlainTextRenderConfig>

{
    /// @brief Text renderer configuration
    using Config = internal::PlainTextRenderConfig;

    using mixin::Configurable<Config>::Configurable;

    /// @brief Helper to write character with cursor tracking
    void writeChar(char ch) noexcept {
        if (_buffer.full() or _cursor.row >= this->config().rows_total) { return; }

        if (ch == '\n') {
            _cursor.newline();
            (void) _buffer.push(ch);
            return;
        }

        if (not _cursor.canWrite(this->config().row_max_length)) { return; }

        _cursor.advance(1, this->config().row_max_length);
        (void) _buffer.push(ch);
    }

    /// @brief Write string with cursor tracking
    void writeString(memory::StringView str) noexcept {
        for (char ch: str) {
            writeChar(ch);
        }
    }

    void writeReal(f64 real, u8 rounding) noexcept {
        memory::StaticString<24> temp;// Enough for double with precision
        (void) temp.append(real, rounding);
        writeString(temp.view());
    }

private:
    memory::StaticString<N> _buffer{};///< Output buffer for rendered text
    internal::PlainTextRenderCursor _cursor{};

    KF_IMPL(Render<PlainTextRender<N>>);

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept {
        // Subtract 1 for title row
        if (this->config().rows_total > _cursor.row + 1) {
            return this->config().rows_total - _cursor.row - 1;
        } else {
            return 0;
        }
    }

    void beginFrameImpl() noexcept {
        _buffer.clear();
        _cursor.reset();
    }

    void endFrameImpl() noexcept {
        this->invoke(_buffer.view());
    }

    void titleImpl(memory::StringView title) noexcept {
        if (this->config().title_centered) {
            const auto spaces = kf::max(0, (int(this->config().row_max_length) - int(title.size())) / 2);
            for (int i = 0; i < spaces; i += 1) {
                writeChar(' ');
            }
        }
        writeString(title);
        writeChar('\n');
    }

    void beginWidgetImpl(usize, bool is_focused, const Style &) noexcept {
        if (is_focused) {
            writeString("* ");
        }
    }

    void endWidgetImpl() noexcept {
        writeChar('\n');
    }

    void checkboxImpl(bool enabled) noexcept {
        constexpr memory::StringView on{"==[ 1 ]"}, off{"[ 0 ]--"};
        writeString(enabled ? on : off);
    }

    template<typename T> void sliderImpl(const T &slider_value, const Range<T> &range, Placement placement) noexcept {
        // Plain text does not support only 'show inside' placement yet
        if (Placement::Hidden != placement) {
            this->value(slider_value);
            writeChar(' ');
        }

        writeChar('[');
        const usize start_col = _cursor.col;
        const usize inner_width = this->config().row_max_length - start_col - 1;// -1 for closing char
        const usize fill = (slider_value - range.start) * inner_width / (range.end - range.start);

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

    void valueImpl(NoneType) noexcept {
        writeString(memory::StringView{"none"});
    }

    void valueImpl(char c) noexcept {
        writeChar(c);
    }

    void valueImpl(memory::StringView str) noexcept {
        writeString(str);
    }

    template<usize M> void valueImpl(const memory::StaticString<M> &str) noexcept {
        writeString(str.view());
    }

    void valueImpl(const char *str) noexcept {
        writeString(memory::StringView{str});
    }

    void valueImpl(bool b) noexcept {
        constexpr memory::StringView label_true{"true"}, label_false{"false"};
        writeString(b ? label_true : label_false);
    }

    void valueImpl(i32 integer) noexcept {
        memory::StaticString<12> temp;// Enough for 32-bit int
        (void) temp.append(integer);
        writeString(temp.view());
    }

    void valueImpl(f32 real) noexcept {
        writeReal(static_cast<f64>(real), this->config().float_places);
    }

    void valueImpl(f64 real) noexcept {
        writeReal(real, this->config().double_places);
    }

    // Semantic color (No-Op in plain text)

    void setForegroundImpl(Color) noexcept {}

    void setBackgroundImpl(Color) noexcept {}

    // Decoration rendering

    void decorationImpl(Decoration decoration) noexcept {
        switch (decoration) {
            case Decoration::Arrow:
                writeString("-> ");
                return;

            case Decoration::Colon:
                writeString(": ");

            default:
                break;
        }
    }

    void beginBlockImpl(Block block_type) noexcept {
        writeChar((block_type == Block::Standart) ? '[' : '<');
    }

    void endBlockImpl(Block block_type) noexcept {
        writeChar((block_type == Block::Standart) ? ']' : '>');
    }
};

}// namespace kf::ui::render