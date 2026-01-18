#pragma once

// for avr capability
#include <math.h>// NOLINT(*-deprecated-headers)

#include <kf/aliases.hpp>
#include <kf/array.hpp>
#include <kf/attributes.hpp>
#include <kf/fn.hpp>
#include <kf/slice.hpp>

#include "kf/ui/Render.hpp"

namespace kf {
namespace ui {

/// @brief Система отрисовки простым текстом
struct TextRender : Render<TextRender> {
    friend struct Render<TextRender>;

    /// @brief Единица измерения текстового интерфейса в глифах
    using GlyphUnit = u8;

    /// @brief Настройки рендера
    struct Settings {
        using RenderHandler = kf::fn<void(const kf::slice<const u8> &)>;

        static constexpr auto rows_default{4};
        static constexpr auto cols_default{16};

        /// @brief Обработчик отрисовки
        RenderHandler on_render_finish{nullptr};

        /// @brief буфер вывода
        kf::slice<u8> buffer{};

        /// @brief Кол-во строк
        GlyphUnit rows_total{rows_default};

        /// @brief Максимальная длина строки
        GlyphUnit row_max_length{cols_default};
    };

    Settings settings{};

private:
    usize buffer_cursor{0};
    GlyphUnit cursor_row{0}, cursor_col{0};
    bool contrast_mode{false};

    kf_nodiscard usize widgetsAvailableImpl() const {
        return settings.rows_total - cursor_row;
    }

    void prepareImpl() {
        buffer_cursor = 0;
    }

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

    void titleImpl(const char *title) {
        (void) print(title);
        (void) write('\n');
    }

    void stringImpl(const char *str) {
        (void) print(str);
    }

    void numberImpl(i32 integer) {
        (void) print(integer);
    }

    void numberImpl(f64 real, u8 rounding) {
        (void) print(real, rounding);
    }

    void arrowImpl() {
        (void) write('-');
        (void) write('>');
        (void) write(' ');
    }

    void colonImpl() {
        (void) write(':');
        (void) write(' ');
    }

    void contrastBeginImpl() {
        (void) write(0x81);
        contrast_mode = true;
    }

    void contrastEndImpl() {
        (void) write(0x80);
        contrast_mode = false;
    }

    void blockBeginImpl() {
        (void) write('[');
    }

    void blockEndImpl() {
        (void) write(']');
    }

    void variableBeginImpl() {
        (void) write('<');
    }

    void variableEndImpl() {
        (void) write('>');
    }

    void widgetBeginImpl(usize) {}

    void widgetEndImpl() {
        (void) write('\n');
    }

    // help methods...

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
