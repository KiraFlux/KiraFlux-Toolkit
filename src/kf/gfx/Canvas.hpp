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

namespace kf::gfx {

/// @brief Drawing context with graphics primitives and text rendering
/// @tparam F Pixel format for canvas operations
template<typename F> struct Canvas {
    using PixelImpl = F;
    using ColorType = typename F::ColorType;
    using PaletteType = Palette<F>;///<Color Palette

private:
    static constexpr ColorType default_foreground_color{PaletteType::ansiColor(PaletteType::Ansi::WhiteBright)};
    static constexpr ColorType default_background_color{PaletteType::ansiColor(PaletteType::Ansi::Black)};

    image::DynamicImage<F> frame;///< Target drawing surface
    const Font *current_font;    ///< Currently selected font
    ColorType foreground_color;  ///< Drawing color
    ColorType background_color;  ///< Background/fill color
    bool auto_next_line;         ///< Automatically wrap text to next line

public:
    explicit Canvas(
        const image::DynamicImage<F> &frame,
        const Font &font = Font::blank(),
        ColorType foreground = default_foreground_color,
        ColorType background = default_background_color) noexcept :
        frame{frame},
        current_font{&font},
        foreground_color{foreground},
        background_color{background},
        auto_next_line{false} {}

    /// @brief Default constructor - creates invalid canvas
    explicit Canvas() noexcept :
        frame{},
        current_font{&Font::blank()},
        foreground_color{default_foreground_color},
        background_color{default_background_color},
        auto_next_line{false} {}

    /// @brief Creates validated sub-canvas within current bounds
    Result<Canvas, typename image::DynamicImage<F>::Error> sub(
        Pixels sub_width, Pixels sub_height,
        Pixels sub_offset_x, Pixels sub_offset_y) noexcept {
        const auto frame_result = frame.sub(sub_width, sub_height, sub_offset_x, sub_offset_y);
        if (frame_result.isOk()) {
            return {Canvas{frame_result.ok().value(), *current_font, foreground_color, background_color}};
        } else {
            return {frame_result.error().value()};
        }
    }

    /// @brief Creates sub-canvas without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    Canvas subUnchecked(
        Pixels width, Pixels height,
        Pixels offset_x, Pixels offset_y) noexcept {
        return Canvas{
            frame.subUnchecked(width, height, offset_x, offset_y),
            *current_font,
            foreground_color,
            background_color};
    }

    // Properties

    /// @brief Get canvas width in pixels
    [[nodiscard]] Pixels width() const noexcept { return frame.width(); }

    /// @brief Get canvas height in pixels
    [[nodiscard]] Pixels height() const noexcept { return frame.height(); }

    /// @brief Get maximum valid X coordinate (rightmost pixel)
    [[nodiscard]] Pixels maxX() const noexcept { return frame.maxX(); }

    /// @brief Get maximum valid Y coordinate (bottommost pixel)
    [[nodiscard]] Pixels maxY() const noexcept { return frame.maxY(); }

    /// @brief Get horizontal center coordinate
    [[nodiscard]] Pixels centerX() const noexcept { return static_cast<Pixels>(maxX() / 2); }

    /// @brief Get vertical center coordinate
    [[nodiscard]] Pixels centerY() const noexcept { return static_cast<Pixels>(maxY() / 2); }

    /// @brief Get tab width based on current font (4 character widths)
    [[nodiscard]] Pixels tabWidth() const noexcept { return static_cast<Pixels>(current_font->widthTotal() * 4); }

    /// @brief Get current font glyph width
    [[nodiscard]] Pixels glyphWidth() const noexcept { return current_font->widthTotal(); }

    /// @brief Get current font glyph height
    [[nodiscard]] Pixels glyphHeight() const noexcept { return current_font->heightTotal(); }

    /// @brief Get canvas width in glyphs
    [[nodiscard]] u8 widthInGlyphs() const noexcept { return width() / glyphWidth(); }

    /// @brief Get canvas height in glyphs
    [[nodiscard]] u8 heightInGlyphs() const noexcept { return height() / glyphHeight(); }

    /// Current Foreground color
    [[nodiscard]] ColorType foreground() const noexcept { return foreground_color; }

    /// Current Background color
    [[nodiscard]] ColorType background() const noexcept { return background_color; }

    // Control

    /// @brief Set current font for text rendering
    void setFont(const Font &font) noexcept { current_font = &font; }

    /// @brief Set background color for fill and text operations
    void setBackground(ColorType color) noexcept { background_color = color; }

    /// @brief Set foreground color for drawing operations
    void setForeground(ColorType color) noexcept { foreground_color = color; }

    /// @brief Swap foreground and background colors
    void swapColors() noexcept { std::swap(foreground_color, background_color); }

    /// @brief Enable/disable automatic text wrapping to next line
    void setAutoNextLine(bool enable) noexcept { auto_next_line = enable; }

    /// @brief Split canvas into weighted sub-canvases
    template<usize N> Array<Canvas, N> split(Array<usize, N> weights, bool horizontal = true) noexcept {
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

        Array<Canvas, N> result;
        for (usize i = 0; i < N; i += 1) {
            Pixels size = (remaining * weights[i]) / total_weight;
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
        frame.fill(background_color);
    }

    /// @brief Draw single pixel at specified coordinates
    void dot(Pixels x, Pixels y) const noexcept {
        frame.setPixel(x, y, foreground_color);
    }

    /// @brief Draw static image at specified position
    /// @param x Left position
    /// @param y Top position
    template<Pixels W, Pixels H> void image(Pixels x, Pixels y, const image::StaticImage<F, W, H> &image) noexcept {
        PixelImpl::copy(
            image.buffer(), image.width(), image.height(),
            frame.buffer(), frame.stride(),
            frame.toAbsoluteX(x), frame.toAbsoluteY(y));
    }

    /// @brief Draw line (x0, y0), (x1, y1) between two points
    void line(Pixels x0, Pixels y0, Pixels x1, Pixels y1) const noexcept {
        if (x0 == x1) {
            if (y0 == y1) {
                frame.setPixel(x0, y0, foreground_color);
            } else {
                drawLineVertical(x0, y0, y1, foreground_color);
            }
            return;
        }

        if (y0 == y1) {
            drawLineHorizontal(x0, y0, x1, foreground_color);
            return;
        }

        const auto dx = static_cast<Pixels>(std::abs(x1 - x0));
        const auto dy = static_cast<Pixels>(-std::abs(y1 - y0));
        const auto sx = (x0 < x1) ? 1 : -1;
        const auto sy = (y0 < y1) ? 1 : -1;
        auto error = dx + dy;

        while (true) {
            frame.setPixel(x0, y0, foreground_color);
            if (x0 == x1 and y0 == y1) { break; }

            const auto double_error = 2 * error;
            if (double_error >= dy) {
                if (x0 == x1) { break; }
                error += dy;
                x0 = static_cast<Pixels>(x0 + sx);
            }
            if (double_error <= dx) {
                if (y0 == y1) { break; }
                error += dx;
                y0 = static_cast<Pixels>(y0 + sy);
            }
        }
    }

    /// @brief Draw rectangle (filled or outline)
    void rect(Pixels x0, Pixels y0, Pixels x1, Pixels y1, bool fill) noexcept {
        if (x0 > x1) { std::swap(x0, x1); }
        if (y0 > y1) { std::swap(y0, y1); }

        if (fill) {
            frame.fill(x0, y0, x1, y1, foreground_color);
        } else {
            // Outline
            drawLineHorizontal(x0, y0, x1, foreground_color);
            drawLineHorizontal(x0, y1, x1, foreground_color);
            y0 += 1;
            drawLineVertical(x0, y0, y1, foreground_color);
            drawLineVertical(x1, y0, y1, foreground_color);
        }
    }

    /// @brief Draw circle (filled or outline)
    void circle(Pixels cx, Pixels cy, Pixels r, bool fill) noexcept {
        if (r < 0) { return; }

        if (fill) {
            // Fill circle
            const auto r_squared = r * r;
            for (auto y = -r; y <= r; y += 1) {
                const int y_squared = y * y;
                const auto width = static_cast<int>(std::sqrt(r_squared - y_squared));

                // todo lineH
                for (auto x = -width; x <= width; x += 1) {
                    frame.setPixel(static_cast<Pixels>(cx + x), static_cast<Pixels>(cy + y), foreground_color);
                }
            }
        } else {
            // Outline circle - modified Bresenham algorithm
            Pixels x = r;
            Pixels y = 0;
            int err = 0;

            while (x >= y) {
                // Draw 8 symmetric points
                drawCirclePoints(cx, cy, x, y, foreground_color);

                // Avoid drawing duplicate points when x == y
                if (x != y) {
                    drawCirclePoints(cx, cy, y, x, foreground_color);
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
    void text(Pixels start_x, Pixels start_y, const char *text) noexcept {
        Pixels cursor_x = start_x;
        Pixels cursor_y = start_y;
        const u8 font_width = current_font->glyph_width;
        const u8 font_height = current_font->glyph_height;
        const u8 font_total_height = current_font->heightTotal();
        ColorType current_foreground_color = foreground_color;
        ColorType current_background_color = background_color;

        for (; *text != '\0'; ++text) {
            switch (*text) {
                case '\x80': {
                    current_foreground_color = foreground_color;
                    current_background_color = background_color;
                    continue;
                }

                case '\x81': {
                    current_foreground_color = background_color;
                    current_background_color = foreground_color;
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
                    current_foreground_color = PaletteType::ansiColor(static_cast<typename PaletteType::Ansi>(*text));
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
                    current_background_color = PaletteType::ansiColor(static_cast<typename PaletteType::Ansi>(*text));
                    continue;
                }

                case '\n': {
                    clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                    cursor_x = start_x;
                    cursor_y = static_cast<Pixels>(cursor_y + font_total_height);
                    continue;
                }

                case '\t': {
                    const auto tab_width = tabWidth();
                    const auto new_x = static_cast<Pixels>(((cursor_x / tab_width) + 1) * tab_width);
                    clearLineSegment(cursor_x, cursor_y, new_x, current_background_color);
                    cursor_x = new_x;
                    continue;
                }
            }

            if (cursor_x > static_cast<Pixels>(width() - font_width)) {
                clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                if (auto_next_line) {
                    cursor_x = start_x;
                    cursor_y = static_cast<Pixels>(cursor_y + font_total_height);
                } else {
                    return;
                }
            }

            if (cursor_y > static_cast<Pixels>(height() - font_height)) { return; }

            drawGlyph(cursor_x, cursor_y, current_font->getGlyph(*text), current_foreground_color, current_background_color);

            cursor_x = static_cast<Pixels>(cursor_x + font_width);
            if (cursor_x < width()) {
                drawLineVertical(
                    cursor_x,
                    cursor_y,
                    static_cast<Pixels>(cursor_y + font_height),
                    current_background_color);
            }
            cursor_x = static_cast<Pixels>(cursor_x + 1);
        }
    }

private:
    // Drawing API backend

    /// @brief Clear rectangular line segment with background color
    void clearLineSegment(Pixels cursor_x, Pixels cursor_y, Pixels end_x, ColorType color) noexcept {
        if (cursor_x < end_x) {
            frame.fill(
                cursor_x, cursor_y,
                end_x, current_font->heightTotal() + cursor_y,
                color);
        }
    }

    /// @brief Draw horizontal line (optimized)
    void drawLineHorizontal(Pixels x0, Pixels y, Pixels x1, ColorType color) const noexcept {
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        frame.fill(x0, y, x1, y, color);
    }

    /// @brief Draw vertical line (optimized)
    void drawLineVertical(Pixels x, Pixels y0, Pixels y1, ColorType color) const noexcept {
        if (y0 > y1) { std::swap(y0, y1); }
        frame.fill(x, y0, x, y1, color);
    }

    /// @brief Draw 8 symmetric points for circle outline
    void drawCirclePoints(Pixels cx, Pixels cy, Pixels dx, Pixels dy, ColorType color) const noexcept {
        frame.setPixel(static_cast<Pixels>(cx + dx), static_cast<Pixels>(cy + dy), color);
        frame.setPixel(static_cast<Pixels>(cx + dy), static_cast<Pixels>(cy + dx), color);
        frame.setPixel(static_cast<Pixels>(cx - dy), static_cast<Pixels>(cy + dx), color);
        frame.setPixel(static_cast<Pixels>(cx - dx), static_cast<Pixels>(cy + dy), color);
        frame.setPixel(static_cast<Pixels>(cx - dx), static_cast<Pixels>(cy - dy), color);
        frame.setPixel(static_cast<Pixels>(cx - dy), static_cast<Pixels>(cy - dx), color);
        frame.setPixel(static_cast<Pixels>(cx + dy), static_cast<Pixels>(cy - dx), color);
        frame.setPixel(static_cast<Pixels>(cx + dx), static_cast<Pixels>(cy - dy), color);
    }

    /// @brief Draw font glyph at specified position
    /// @param x Left position
    /// @param y Top position
    /// @param glyph Pointer to glyph bitmap data
    void drawGlyph(Pixels x, Pixels y, const u8 *glyph, ColorType color_on, ColorType color_off) noexcept {
        if (nullptr == glyph) {
            // Draw box for unknown character
            const auto x1 = static_cast<Pixels>(x + current_font->glyph_width - 1);
            const auto y1 = static_cast<Pixels>(y + current_font->glyph_height - 1);

            drawLineHorizontal(x, y, x1, color_on);
            drawLineHorizontal(x, y1, x1, color_on);
            drawLineVertical(x, y, y1, color_on);
            drawLineVertical(x1, y, y1, color_on);
            return;
        }

        const u8 font_width = current_font->glyph_width;
        const u8 font_height = current_font->glyph_height;

        for (u8 col = 0; col < font_width; ++col) {
            const auto pixel_x = static_cast<Pixels>(x + col);
            const u8 glyph_byte = glyph[col];

            for (u8 row = 0; row < font_height; row += 1) {
                const auto color = (glyph_byte >> row) & 1 ? color_on : color_off;
                frame.setPixel(pixel_x, static_cast<Pixels>(y + row), color);
            }

            frame.setPixel(pixel_x, static_cast<Pixels>(y + font_height), color_off);
        }
    }
};

}// namespace kf::gfx
