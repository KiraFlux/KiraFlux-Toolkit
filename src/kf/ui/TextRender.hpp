// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

// for avr capability
#include <math.h>// NOLINT(*-deprecated-headers)

#include "kf/Function.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/Slice.hpp"
#include "kf/ui/Render.hpp"

namespace kf {// NOLINT(*-concat-nested-namespaces) // for c++11 capability
namespace ui {

/// @brief Text-based UI rendering system for terminal/console output
/// @note Implements Render CRTP interface for character-based display
struct TextRender : Render<TextRender> {
    friend struct Render<TextRender>;

    using GlyphUnit = u8;///< Text interface measurement unit in glyphs

    /// @brief Text renderer configuration settings
    struct Settings {
        using RenderHandler = Function<void(Slice<const u8>)>;///< Render completion callback type

        static constexpr auto rows_default{4}; ///< Default row count
        static constexpr auto cols_default{16};///< Default column count

        RenderHandler on_render_finish{nullptr};///< Callback invoked when rendering completes
        Slice<u8> buffer{};                     ///< Output buffer for rendered text
        GlyphUnit rows_total{rows_default};     ///< Total available rows in display
        GlyphUnit row_max_length{cols_default}; ///< Maximum characters per row
    };

    Settings settings{};///< Current renderer configuration

private:
    usize buffer_cursor{0};   ///< Current position in output buffer
    GlyphUnit cursor_row{0};  ///< Current row position in virtual display
    GlyphUnit cursor_col{0};  ///< Current column position in current row
    bool contrast_mode{false};///< Current contrast mode state

    /// @brief Calculate remaining widget capacity
    /// @return Number of widgets that can still be rendered
    kf_nodiscard usize widgetsAvailableImpl() const {
        return settings.rows_total - cursor_row;
    }

    /// @brief Prepare renderer for new frame
    void prepareImpl() {
        buffer_cursor = 0;
    }

    /// @brief Finalize current frame and invoke callback
    void finishImpl() {
        if (nullptr == settings.buffer.data()) {
            return;
        }

        cursor_row = 0;
        cursor_col = 0;
        settings.buffer.data()[buffer_cursor - 1] = '\0';

        if (settings.on_render_finish) {
            settings.on_render_finish({settings.buffer.data(), buffer_cursor});
        }
    }

    /// @brief Render page title line
    /// @param title Title text to display
    void titleImpl(const char *title) {
        (void) print(title);
        (void) write('\n');
    }

    /// @brief Render text string
    /// @param str String to display
    void stringImpl(const char *str) {
        (void) print(str);
    }

    /// @brief Render integer value
    /// @param integer Integer to display
    void numberImpl(i32 integer) {
        (void) print(integer);
    }

    /// @brief Render floating-point value
    /// @param real Floating-point number to display
    /// @param rounding Number of decimal places to show
    void numberImpl(f64 real, u8 rounding) {
        (void) print(real, rounding);
    }

    /// @brief Render arrow indicator (->)
    void arrowImpl() {
        (void) write('-');
        (void) write('>');
        (void) write(' ');
    }

    /// @brief Render colon separator (: )
    void colonImpl() {
        (void) write(':');
        (void) write(' ');
    }

    /// @brief Begin high-contrast text region (special character 0x81)
    void beginContrastImpl() {
        (void) write(0x81);
        contrast_mode = true;
    }

    /// @brief End high-contrast text region (special character 0x80)
    void endContrastImpl() {
        (void) write(0x80);
        contrast_mode = false;
    }

    /// @brief Begin standard block marker ([)
    void beginBlockImpl() {
        (void) write('[');
    }

    /// @brief End standard block marker (])
    void endBlockImpl() {
        (void) write(']');
    }

    /// @brief Begin alternative block marker (<)
    void beginAltBlockImpl() {
        (void) write('<');
    }

    /// @brief End alternative block marker (>)
    void endAltBlockImpl() {
        (void) write('>');
    }

    /// @brief Begin widget rendering (no-op in text renderer)
    /// @param index Widget index (unused)
    void beginWidgetImpl(usize) {}

    /// @brief End widget rendering (newline)
    void endWidgetImpl() {
        (void) write('\n');
    }

    /// @brief Print null-terminated string to buffer
    /// @param str String to print (nullptr prints "nullptr")
    /// @return Number of characters written
    kf_nodiscard usize print(const char *str) {
        if (nullptr == str) {
            str = "nullptr";
        }

        usize written{0};

        while (*str != '\x00') {
            written += write(*str);
            str += 1;
        }

        return written;
    }

    /// @brief Print integer to buffer
    /// @param integer Integer value to print
    /// @return Number of characters written
    kf_nodiscard usize print(i32 integer) {
        if (integer == 0) {
            return write('0');
        }

        usize written{0};

        if (integer < 0) {
            integer = -integer;
            written += write('-');
        }

        char digits_buffer[12];

        auto digits_total{0};
        while (integer > 0) {
            const auto base = 10;

            digits_buffer[digits_total] = static_cast<char>(integer % base + '0');
            digits_total += 1;
            integer /= base;
        }

        for (auto i = digits_total - 1; i >= 0; i -= 1) {
            written += write(digits_buffer[i]);
        }

        return written;
    }

    /// @brief Print floating-point number to buffer
    /// @param real Floating-point value to print
    /// @param rounding Number of decimal places to show
    /// @return Number of characters written
    kf_nodiscard usize print(f64 real, u8 rounding) {
        if (isnan(real)) {
            return print("nan");
        }

        if (isinf(real)) {
            return print("inf");
        }

        usize written{0};

        if (real < 0) {
            real = -real;
            written += write('-');
        }

        written += print(i32(real));

        if (rounding > 0) {
            written += write('.');

            auto fractional = real - i32(real);

            for (auto i = 0; i < rounding; i += 1) {
                const auto base = 10;

                fractional *= base;
                const auto digit = u8(fractional);
                written += write('0' + digit);
                fractional -= digit;
            }
        }

        return written;
    }

    /// @brief Write single character to buffer
    /// @param c Character to write
    /// @return 1 if character written, 0 otherwise
    /// @note Handles line wrapping, row limits, and contrast mode
    kf_nodiscard usize write(u8 c) {
        if (buffer_cursor >= settings.buffer.size()) {
            return 0;
        }

        if (cursor_row >= settings.rows_total) {
            return 0;
        }

        if ('\n' == c) {
            cursor_row += 1;
            cursor_col = 0;
        } else {
            if (cursor_col >= settings.row_max_length) {
                if (contrast_mode and buffer_cursor < settings.buffer.size()) {
                    settings.buffer.data()[buffer_cursor] = 0x80;
                    buffer_cursor += 1;
                    contrast_mode = false;
                }
                return 0;
            }
            cursor_col += 1;
        }
        settings.buffer.data()[buffer_cursor] = c;
        buffer_cursor += 1;
        return 1;
    }
};

}// namespace ui
}// namespace kf