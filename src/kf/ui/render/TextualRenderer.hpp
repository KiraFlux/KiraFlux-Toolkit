// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    ui/render/TextualRenderer.hpp
/// @brief   Plain‑text UI renderer for terminals/consoles.

#pragma once

#include "kf/Function.hpp"
#include "kf/Slice.hpp"
#include "kf/String.hpp"
#include "kf/StringView.hpp"
#include "kf/core.hpp"
#include "kf/math.hpp"

#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Resettable.hpp"

#include "kf/ui/Block.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/Layout.hpp"
#include "kf/ui/render/Renderer.hpp"

namespace kf::internal {

using Glyph = u8;///< Text interface measurement unit in glyphs

struct TextualRendererConfig : mixin::Resettable<TextualRendererConfig> {
    Glyph row_max_length;///< Maximum characters per row
    Glyph rows_total;    ///< Total available rows in display
    Glyph float_places;  ///< Decimal places for float
    Glyph double_places; ///< Decimal places for double
    Glyph offset_size;   ///< Spaces per one offset value
    bool title_centered; ///< Render Title centered

private:
    KF_IMPL_RESETTABLE(TextualRendererConfig);
    constexpr void resetImpl() noexcept {
        row_max_length = 16;
        rows_total = 4;
        float_places = 2;
        double_places = 4;
        offset_size = 2;
        title_centered = true;
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
struct TextualRenderer :

    Renderer<TextualRenderer>,
    mixin::Callbacked<void(StringView)>,
    mixin::Configured<internal::TextualRendererConfig>

{
    /// @brief Text renderer configuration
    using Config = internal::TextualRendererConfig;

    explicit constexpr TextualRenderer(Config const &config, Slice<char> source) noexcept :
        mixin::Configured<Config>::Configured{config}, _buffer{source} {}

    /// @brief Helper to write character with cursor tracking
    void writeChar(char ch) noexcept {
        if (_buffer.full() or _cursor.row >= this->config().rows_total) { return; }

        if (ch == '\n') {
            _cursor.newline();
            _buffer.append(ch);
            return;
        }

        if (_cursor.canWrite(this->config().row_max_length)) {
            _cursor.advance(1, this->config().row_max_length);
            _buffer.append(ch);
            return;
        }
    }

    /// @brief Write string with cursor tracking
    void writeString(StringView str) noexcept {
        for (char ch: str) {
            writeChar(ch);
        }
    }

    void writeReal(f64 real, u8 rounding) noexcept {
        char buffer[24];

        String temp{{buffer}};
        temp.append(real, rounding);

        writeString(temp.view());
    }

    void fillSpace(usize spaces) noexcept {
        for (usize i = 0; i < spaces; i += 1) {
            writeChar(' ');
        }
    }

private:
    String _buffer;// TODO: implement TUI string with cursor logic
    internal::PlainTextRendererCursor _cursor{};

    Layout _layout{Layout::Vertical};

    KF_IMPL_RENDERER(TextualRenderer);

    void beginFrameImpl() noexcept {
        _buffer.reset();
        _cursor.reset();
    }

    void endFrameImpl() noexcept {
        this->invoke(_buffer.view());
    }

    usize beginPageImpl(StringView title, Layout layout) noexcept {
        _layout = layout;

        if (this->config().title_centered) {
            fillSpace(math::max(0, (int(this->config().row_max_length) - int(title.length())) / 2));
        }
        writeString(title);
        writeChar('\n');

        return this->config().rows_total - _cursor.row;
    }

    void endPageImpl() noexcept {}

    void beginWidgetImpl(usize, bool is_focused, usize offset) noexcept {
        writeChar(is_focused ? '*' : ' ');
        writeChar(' ');

        if (_layout == Layout::Vertical) {
            fillSpace(this->config().offset_size * offset);
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
        constexpr StringView on{"==[ 1 ]"}, off{"[ 0 ]--"};
        writeString(enabled ? on : off);
    }

    void sliderImpl(f32 fill) noexcept {
        usize const start_col = _cursor.col;
        usize const inner_width = this->config().row_max_length - start_col - 1;// -1 for closing char
        usize const fill_chars = fill * inner_width;

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
        writeString("none");
    }

    void valueImpl(auto value) noexcept {

        using T = decltype(value);

        if constexpr (std::is_same_v<T, char>) {
            writeChar(value);
        } else if constexpr (std::is_same_v<bool, T>) {
            constexpr StringView label_true{"true"}, label_false{"false"};
            writeString(value ? label_true : label_false);
        } else if constexpr (std::is_same_v<f64, T>) {
            writeReal(value, this->config().double_places);
        } else if constexpr (std::is_same_v<f32, T>) {
            writeReal(value, this->config().float_places);
        } else if constexpr (std::is_integral_v<T>) {

            char buffer[21];

            String temp{{buffer}};
            temp.append(static_cast<i64>(value));

            writeString(temp.view());
        }
    }

    void valueImpl(StringView str) noexcept {
        writeString(str);
    }

    // Semantic color (No-Op in plain text)

    void setForegroundImpl(Color) noexcept {}

    void setBackgroundImpl(Color) noexcept {}

    // Properties

    constexpr Layout getLayoutImpl() const noexcept {
        return _layout;
    }
};

}// namespace kf::ui::render