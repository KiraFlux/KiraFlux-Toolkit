// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Result.hpp"
#include "kf/memory/Array.hpp"

#include "kf/gfx/Font.hpp"
#include "kf/gfx/Palette.hpp"
#include "kf/image/DynamicImage.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/pixel/Tag.hpp"

namespace kf::gfx {

/// @brief Drawing context with graphics primitives and text rendering
/// @tparam P Pixel format for canvas operations
template<typename P> struct Canvas {
    kf_crtp_check(P, pixel::Tag);

    using PixelImpl = P;
    using ColorType = typename PixelImpl::ColorType;
    using PaletteType = Palette<PixelImpl>;

private:
    static constexpr ColorType default_foreground_color{PaletteType::bright_white};
    static constexpr ColorType default_background_color{PaletteType::black};

    image::DynamicImage<P> _frame;///< Target drawing surface
    const Font *_active_font;     ///< Currently selected font (not nullptr)
    ColorType _foreground;        ///< Drawing color
    ColorType _background;        ///< Background/fill color
    bool _auto_next_line;         ///< Automatically wrap text to next line

public:
    explicit Canvas(
        const image::DynamicImage<P> &frame,
        const Font &font = Font::blank(),
        ColorType foreground = default_foreground_color,
        ColorType background = default_background_color) noexcept :
        _frame{frame},
        _active_font{&font},
        _foreground{foreground},
        _background{background},
        _auto_next_line{false} {}

    /// @brief Default constructor - creates invalid canvas
    explicit Canvas() noexcept :
        _frame{},
        _active_font{&Font::blank()},
        _foreground{default_foreground_color},
        _background{default_background_color},
        _auto_next_line{false} {}

    /// @brief Creates validated sub-canvas within current bounds
    Result<Canvas, typename image::DynamicImage<PixelImpl>::Error> sub(
        math::Pixels sub_width, math::Pixels sub_height,
        math::Pixels sub_offset_x, math::Pixels sub_offset_y) noexcept {
        const auto frame_result = _frame.sub(sub_width, sub_height, sub_offset_x, sub_offset_y);
        if (frame_result.isOk()) {
            return {Canvas{frame_result.ok().value(), *_active_font, _foreground, _background}};
        } else {
            return {frame_result.error().value()};
        }
    }

    /// @brief Creates sub-canvas without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    Canvas subUnchecked(
        math::Pixels width, math::Pixels height,
        math::Pixels offset_x, math::Pixels offset_y) noexcept {
        return Canvas{
            _frame.subUnchecked(width, height, offset_x, offset_y),
            *_active_font,
            _foreground,
            _background};
    }

    // Properties

    /// @brief Get canvas width in pixels
    [[nodiscard]] math::Pixels width() const noexcept { return _frame.width(); }

    /// @brief Get canvas height in pixels
    [[nodiscard]] math::Pixels height() const noexcept { return _frame.height(); }

    /// @brief Get maximum valid X coordinate (rightmost pixel)
    [[nodiscard]] math::Pixels maxX() const noexcept { return _frame.maxX(); }

    /// @brief Get maximum valid Y coordinate (bottommost pixel)
    [[nodiscard]] math::Pixels maxY() const noexcept { return _frame.maxY(); }

    /// @brief Get horizontal center coordinate
    [[nodiscard]] math::Pixels centerX() const noexcept { return static_cast<math::Pixels>(maxX() / 2); }

    /// @brief Get vertical center coordinate
    [[nodiscard]] math::Pixels centerY() const noexcept { return static_cast<math::Pixels>(maxY() / 2); }

    /// @brief Get tab width based on current font (4 character widths)
    [[nodiscard]] math::Pixels tabWidth() const noexcept { return static_cast<math::Pixels>(_active_font->widthTotal() * 4); }

    /// @brief Get current font glyph width
    [[nodiscard]] math::Pixels glyphWidth() const noexcept { return _active_font->widthTotal(); }

    /// @brief Get current font glyph height
    [[nodiscard]] math::Pixels glyphHeight() const noexcept { return _active_font->heightTotal(); }

    /// @brief Get canvas width in glyphs
    [[nodiscard]] u8 widthInGlyphs() const noexcept { return width() / glyphWidth(); }

    /// @brief Get canvas height in glyphs
    [[nodiscard]] u8 heightInGlyphs() const noexcept { return height() / glyphHeight(); }

    /// Current Foreground color
    [[nodiscard]] ColorType foreground() const noexcept { return _foreground; }

    /// @brief Set foreground color for drawing operations
    void foreground(ColorType color) noexcept { _foreground = color; }

    /// Current Background color
    [[nodiscard]] ColorType background() const noexcept { return _background; }

    /// @brief Set background color for fill and text operations
    void background(ColorType color) noexcept { _background = color; }

    /// @brief Set current font for text rendering
    void font(const Font &new_font) noexcept { _active_font = &new_font; }

    /// @brief Enable/disable automatic text wrapping to next line
    void autoNextLine(bool enable) noexcept { _auto_next_line = enable; }

    // Control

    /// @brief Swap foreground and background colors
    void swapColors() noexcept { std::swap(_foreground, _background); }

    /// @brief Split canvas into weighted sub-canvases
    template<usize N> memory::Array<Canvas, N> split(memory::Array<usize, N> weights, bool horizontal = true) noexcept {
        static_assert(N > 0, "Cannot split with zero items");
        for (auto &w: weights) {
            if (w == 0) { w = 1; }
        }

        usize total_weight = 0;
        for (auto w: weights) {
            total_weight += w;
        }

        auto remaining = horizontal ? width() : height();
        auto offset = 0u;

        memory::Array<Canvas, N> result;
        for (usize i = 0; i < N; i += 1) {
            math::Pixels size = (remaining * weights[i]) / total_weight;
            if (i == N - 1) { size = remaining; }

            if (size > 0) {
                if (horizontal) {
                    result[i] = subUnchecked(size, height(), offset, 0);
                } else {
                    result[i] = subUnchecked(width(), size, 0, offset);
                }
            }
            offset += size;
            remaining -= size;
        }
        return result;
    }

    // Drawing API

    /// @brief Fill entire canvas with background color
    void fill() const noexcept {
        _frame.fill(_background);
    }

    /// @brief Draw single pixel at specified coordinates
    void dot(math::Pixels x, math::Pixels y) const noexcept {
        _frame.setPixel(x, y, _foreground);
    }

    /// @brief Draw static image at specified position
    /// @param x Left position
    /// @param y Top position
    template<math::Pixels W, math::Pixels H> void image(math::Pixels x, math::Pixels y, const image::StaticImage<P, W, H> &image) noexcept {
        PixelImpl::copy(
            image.buffer(), image.width(), image.height(),
            _frame.buffer(), _frame.stride(),
            _frame.toAbsoluteX(x), _frame.toAbsoluteY(y));
    }

    /// @brief Draw line (x0, y0), (x1, y1) between two points
    void line(math::Pixels x0, math::Pixels y0, math::Pixels x1, math::Pixels y1) const noexcept {
        if (x0 == x1) {
            if (y0 == y1) {
                _frame.setPixel(x0, y0, _foreground);
            } else {
                drawLineVertical(x0, y0, y1, _foreground);
            }
            return;
        }

        if (y0 == y1) {
            drawLineHorizontal(x0, y0, x1, _foreground);
            return;
        }

        const auto dx = static_cast<math::Pixels>(std::abs(x1 - x0));
        const auto dy = static_cast<math::Pixels>(-std::abs(y1 - y0));
        const auto sx = (x0 < x1) ? 1 : -1;
        const auto sy = (y0 < y1) ? 1 : -1;
        auto error = dx + dy;

        while (true) {
            _frame.setPixel(x0, y0, _foreground);
            if (x0 == x1 and y0 == y1) { break; }

            const auto double_error = 2 * error;
            if (double_error >= dy) {
                if (x0 == x1) { break; }
                error += dy;
                x0 = static_cast<math::Pixels>(x0 + sx);
            }
            if (double_error <= dx) {
                if (y0 == y1) { break; }
                error += dx;
                y0 = static_cast<math::Pixels>(y0 + sy);
            }
        }
    }

    /// @brief Draw rectangle (filled or outline)
    void rect(math::Pixels x0, math::Pixels y0, math::Pixels x1, math::Pixels y1, bool fill) noexcept {
        if (x0 > x1) { std::swap(x0, x1); }
        if (y0 > y1) { std::swap(y0, y1); }

        if (fill) {
            _frame.fill(x0, y0, x1, y1, _foreground);
        } else {
            // Outline
            drawLineHorizontal(x0, y0, x1, _foreground);
            drawLineHorizontal(x0, y1, x1, _foreground);
            y0 += 1;
            drawLineVertical(x0, y0, y1, _foreground);
            drawLineVertical(x1, y0, y1, _foreground);
        }
    }

    /// @brief Draw circle (filled or outline)
    void circle(math::Pixels cx, math::Pixels cy, math::Pixels r, bool fill) noexcept {
        if (r < 0) { return; }

        if (fill) {
            // Fill circle
            const auto r_squared = r * r;
            for (auto y = -r; y <= r; y += 1) {
                const int y_squared = y * y;
                const auto width = static_cast<int>(std::sqrt(r_squared - y_squared));

                // todo lineH
                for (auto x = -width; x <= width; x += 1) {
                    _frame.setPixel(static_cast<math::Pixels>(cx + x), static_cast<math::Pixels>(cy + y), _foreground);
                }
            }
        } else {
            // Outline circle - modified Bresenham algorithm
            math::Pixels x = r;
            math::Pixels y = 0;
            int err = 0;

            while (x >= y) {
                // Draw 8 symmetric points
                drawCirclePoints(cx, cy, x, y, _foreground);

                // Avoid drawing duplicate points when x == y
                if (x != y) {
                    drawCirclePoints(cx, cy, y, x, _foreground);
                }

                y += 1;
                err += 2 * y - 1;

                // Update x if needed
                if (err > 0) {
                    x -= 1;
                    err -= 2 * x + 1;
                }
            }
        }
    }

    /// @brief Draw text at specified position
    /// @details Supports formatting codes:
    ///   \x80 - Normal color mode (text - fg, bg)
    ///   \x81 - Invert color mode (bg, text - fg)
    ///   \x82 - swap bg <-> fg colors
    ///   \xF* - Set custom foreground color (ANSI color index)
    ///   \xB* - Set custom background color (ANSI color index)
    ///   \n - New line
    ///   \t - Tab (4 character widths)
    void text(math::Pixels start_x, math::Pixels start_y, const char *text) noexcept {
        math::Pixels cursor_x = start_x;
        math::Pixels cursor_y = start_y;
        const u8 font_width = _active_font->glyph_width;
        const u8 font_height = _active_font->glyph_height;
        const u8 font_total_height = _active_font->heightTotal();
        ColorType current_foreground_color = _foreground;
        ColorType current_background_color = _background;

        for (; *text != '\0'; ++text) {
            switch (*text) {
                case '\x80': {
                    current_foreground_color = _foreground;
                    current_background_color = _background;
                    continue;
                }

                case '\x81': {
                    current_foreground_color = _background;
                    current_background_color = _foreground;
                    continue;
                }

                case '\x82': {
                    std::swap(current_background_color, current_foreground_color);
                    continue;
                }

                case '\xF0':
                case '\xF1':
                case '\xF2':
                case '\xF3':
                case '\xF4':
                case '\xF5':
                case '\xF6':
                case '\xF7':
                case '\xF8':
                case '\xF9':
                case '\xFA':
                case '\xFB':
                case '\xFC':
                case '\xFD':
                case '\xFE':
                case '\xFF': {
                    current_foreground_color = PaletteType::ansiColor(static_cast<u8>(*text));
                    continue;
                }

                case '\xB0':
                case '\xB1':
                case '\xB2':
                case '\xB3':
                case '\xB4':
                case '\xB5':
                case '\xB6':
                case '\xB7':
                case '\xB8':
                case '\xB9':
                case '\xBA':
                case '\xBB':
                case '\xBC':
                case '\xBD':
                case '\xBE':
                case '\xBF': {
                    current_background_color = PaletteType::ansiColor(static_cast<u8>(*text));
                    continue;
                }

                case '\n': {
                    clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                    cursor_x = start_x;
                    cursor_y = static_cast<math::Pixels>(cursor_y + font_total_height);
                    continue;
                }

                case '\t': {
                    const auto tab_width = tabWidth();
                    const auto new_x = static_cast<math::Pixels>(((cursor_x / tab_width) + 1) * tab_width);
                    clearLineSegment(cursor_x, cursor_y, new_x, current_background_color);
                    cursor_x = new_x;
                    continue;
                }
            }

            if (cursor_x > static_cast<math::Pixels>(width() - font_width)) {
                clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                if (_auto_next_line) {
                    cursor_x = start_x;
                    cursor_y = static_cast<math::Pixels>(cursor_y + font_total_height);
                } else {
                    return;
                }
            }

            if (cursor_y > static_cast<math::Pixels>(height() - font_height)) { return; }

            drawGlyph(cursor_x, cursor_y, _active_font->getGlyph(*text), current_foreground_color, current_background_color);

            cursor_x = static_cast<math::Pixels>(cursor_x + font_width);
            if (cursor_x < width()) {
                drawLineVertical(
                    cursor_x,
                    cursor_y,
                    static_cast<math::Pixels>(cursor_y + font_height),
                    current_background_color);
            }
            cursor_x = static_cast<math::Pixels>(cursor_x + 1);
        }
    }

private:
    // Drawing API backend

    /// @brief Clear rectangular line segment with background color
    void clearLineSegment(math::Pixels cursor_x, math::Pixels cursor_y, math::Pixels end_x, ColorType color) noexcept {
        if (cursor_x < end_x) {
            _frame.fill(
                cursor_x, cursor_y,
                end_x, _active_font->heightTotal() + cursor_y,
                color);
        }
    }

    /// @brief Draw horizontal line (optimized)
    void drawLineHorizontal(math::Pixels x0, math::Pixels y, math::Pixels x1, ColorType color) const noexcept {
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        _frame.fill(x0, y, x1, y, color);
    }

    /// @brief Draw vertical line (optimized)
    void drawLineVertical(math::Pixels x, math::Pixels y0, math::Pixels y1, ColorType color) const noexcept {
        if (y0 > y1) { std::swap(y0, y1); }
        _frame.fill(x, y0, x, y1, color);
    }

    /// @brief Draw 8 symmetric points for circle outline
    void drawCirclePoints(math::Pixels cx, math::Pixels cy, math::Pixels dx, math::Pixels dy, ColorType color) const noexcept {
        _frame.setPixel(static_cast<math::Pixels>(cx + dx), static_cast<math::Pixels>(cy + dy), color);
        _frame.setPixel(static_cast<math::Pixels>(cx + dy), static_cast<math::Pixels>(cy + dx), color);
        _frame.setPixel(static_cast<math::Pixels>(cx - dy), static_cast<math::Pixels>(cy + dx), color);
        _frame.setPixel(static_cast<math::Pixels>(cx - dx), static_cast<math::Pixels>(cy + dy), color);
        _frame.setPixel(static_cast<math::Pixels>(cx - dx), static_cast<math::Pixels>(cy - dy), color);
        _frame.setPixel(static_cast<math::Pixels>(cx - dy), static_cast<math::Pixels>(cy - dx), color);
        _frame.setPixel(static_cast<math::Pixels>(cx + dy), static_cast<math::Pixels>(cy - dx), color);
        _frame.setPixel(static_cast<math::Pixels>(cx + dx), static_cast<math::Pixels>(cy - dy), color);
    }

    /// @brief Draw font glyph at specified position
    /// @param x Left position
    /// @param y Top position
    /// @param glyph Pointer to glyph bitmap data
    void drawGlyph(math::Pixels x, math::Pixels y, const u8 *glyph, ColorType color_on, ColorType color_off) noexcept {
        if (nullptr == glyph) {
            // Draw box for unknown character
            const auto x1 = static_cast<math::Pixels>(x + _active_font->glyph_width - 1);
            const auto y1 = static_cast<math::Pixels>(y + _active_font->glyph_height - 1);

            drawLineHorizontal(x, y, x1, color_on);
            drawLineHorizontal(x, y1, x1, color_on);
            drawLineVertical(x, y, y1, color_on);
            drawLineVertical(x1, y, y1, color_on);
            return;
        }

        const u8 font_width = _active_font->glyph_width;
        const u8 font_height = _active_font->glyph_height;

        for (u8 col = 0; col < font_width; ++col) {
            const auto pixel_x = static_cast<math::Pixels>(x + col);
            const u8 glyph_byte = glyph[col];

            for (u8 row = 0; row < font_height; row += 1) {
                const auto color = (glyph_byte >> row) & 1 ? color_on : color_off;
                _frame.setPixel(pixel_x, static_cast<math::Pixels>(y + row), color);
            }

            _frame.setPixel(pixel_x, static_cast<math::Pixels>(y + font_height), color_off);
        }
    }
};

}// namespace kf::gfx
