// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/Slice.hpp"
#include "kf/algorithm.hpp"
#include "kf/memory/StaticString.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Block.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/Layout.hpp"
#include "kf/ui/render/Renderer.hpp"

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
struct PlainTextRendererCursor {
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
/// @note Implements Renderer CRTP interface for character-based display
struct PlainTextRenderer :

    Renderer<PlainTextRenderer>,
    mixin::Callbacked<void(memory::StringView)>,
    mixin::Configurable<internal::PlainTextRenderConfig>

{
    /// @brief Text renderer configuration
    using Config = internal::PlainTextRenderConfig;

    explicit constexpr PlainTextRenderer(const Config &config, Slice<char> source) noexcept :
        mixin::Configurable<Config>::Configurable{config}, _buffer{source} {}

    /// @brief Helper to write character with cursor tracking
    void writeChar(char ch) noexcept {
        if ((_buffer.size() == _written) or _cursor.row >= this->config().rows_total) { return; }

        if (ch == '\n') {
            _cursor.newline();
            _buffer[_written] = ch;
            _written += 1;
            return;
        }

        if (_cursor.canWrite(this->config().row_max_length)) {
            _cursor.advance(1, this->config().row_max_length);
            _buffer[_written] = ch;
            _written += 1;
            return;
        }
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
    Slice<char> _buffer;
    usize _written{0};
    internal::PlainTextRendererCursor _cursor{};
    Layout _layout{Layout::Vertical};

    KF_IMPL(Renderer<PlainTextRenderer>);

    void beginFrameImpl() noexcept {
        _written = 0;
        _cursor.reset();
    }

    void endFrameImpl() noexcept {
        this->invoke(memory::StringView{_buffer.data(), _written});
    }

    usize beginPageImpl(memory::StringView title, Layout layout) noexcept {
        _layout = layout;

        if (this->config().title_centered) {
            const auto spaces = kf::max(0, (int(this->config().row_max_length) - int(title.size())) / 2);
            for (int i = 0; i < spaces; i += 1) {
                writeChar(' ');
            }
        }
        writeString(title);
        writeChar('\n');

        return this->config().rows_total - _cursor.row;
    }

    void endPageImpl() noexcept {}

    void beginWidgetImpl(usize, bool is_focused) noexcept {
        if (is_focused) {
            writeString("* ");
        }
    }

    void endWidgetImpl() noexcept {
        writeChar((_layout == Layout::Horizontal) ? ' ' : '\n');
    }

    void beginBlockImpl(Block block_type) noexcept {
        writeChar((block_type == Block::Standard) ? '[' : '<');
    }

    void endBlockImpl(Block block_type) noexcept {
        writeChar((block_type == Block::Standard) ? ']' : '>');
    }

    void decorationImpl(Decoration decoration) noexcept {
        switch (decoration) {
            case Decoration::Space:
                writeChar(' ');
                return;

            case Decoration::Colon:
                writeString(": ");
                return;

            case Decoration::LongArrowRight:
                writeString("->");
                return;

            case Decoration::LongArrowLeft:
                writeString("<-");
                return;

            case Decoration::ShortArrowLeft:
                writeChar('>');
                return;

            case Decoration::ShortArrowRight:
                writeChar('<');
                return;

            case Decoration::ShortArrowUp:
                writeChar('A');
                return;

            case Decoration::ShortArrowDown:
                writeChar('V');
                return;

            default:
                break;
        }
    }

    void checkboxImpl(bool enabled) noexcept {
        constexpr memory::StringView on{"==[ 1 ]"}, off{"[ 0 ]--"};
        writeString(enabled ? on : off);
    }

    void sliderImpl(f32 fill) noexcept {
        const usize start_col = _cursor.col;
        const usize inner_width = this->config().row_max_length - start_col - 1;// -1 for closing char
        const usize fill_chars = fill * inner_width;

        writeChar('[');

        for (auto i = 0; i < fill_chars; i += 1) {
            writeChar('=');
        }

        writeChar('@');

        for (auto i = _cursor.col - start_col; i < inner_width; i += 1) {
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
};

}// namespace kf::ui::render