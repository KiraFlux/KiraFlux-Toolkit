// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Result.hpp"
#include "kf/core/attributes.hpp"
#include "kf/core/pixel_traits.hpp"
#include "kf/memory/Array.hpp"

#include "kf/gfx/ColorPalette.hpp"
#include "kf/gfx/DynamicImage.hpp"
#include "kf/gfx/Font.hpp"
#include "kf/gfx/StaticImage.hpp"
#include "ColorPalette.hpp"


namespace kf::gfx {

/// @brief Drawing context with graphics primitives and text rendering
/// @tparam F Pixel format for canvas operations
template<PixelFormat F> struct Canvas {

private:
    using traits = pixel_traits<F>;
    using Palette = ColorPalette<F>;

public:
    using ColorType = typename traits::ColorType;///< Color representation type

private:
    static constexpr ColorType default_foreground_color{Palette::getAnsiColor(Palette::Ansi::WhiteBright)};
    static constexpr ColorType default_background_color{Palette::getAnsiColor(Palette::Ansi::Black)};

    DynamicImage<F> frame;     ///< Target drawing surface
    const Font *current_font;  ///< Currently selected font
    ColorType foreground_color;///< Drawing color
    ColorType background_color;///< Background/fill color
    bool auto_next_line;       ///< Automatically wrap text to next line

public:
    explicit Canvas(
        const DynamicImage<F> &frame,
        const Font &font = Font::blank(),
        ColorType foreground = default_foreground_color,
        ColorType background = default_background_color
    ) noexcept:
        frame{frame},
        current_font{&font},
        foreground_color{foreground},
        background_color{background},
        auto_next_line{false} {}

    /// @brief Default constructor - creates invalid canvas
    explicit Canvas() noexcept:
        frame{},
        current_font{&Font::blank()},
        foreground_color{default_foreground_color},
        background_color{default_background_color},
        auto_next_line{false} {}

    /// @brief Creates validated sub-canvas within current bounds
    /// @param width Sub-canvas width
    /// @param height Sub-canvas height
    /// @param offset_x X offset within current canvas
    /// @param offset_y Y offset within current canvas
    /// @return Sub-canvas or error if out of bounds
    Result<Canvas, typename DynamicImage<F>::Error> sub(
        Pixel width, Pixel height,
        Pixel offset_x, Pixel offset_y
    ) {
        const auto frame_result = frame.sub(width, height, offset_x, offset_y);
        if (frame_result.isOk()) {
            return {Canvas{frame_result.ok().value(), *current_font, foreground_color, background_color}};
        }
        return {frame_result.error().value()};
    }

    /// @brief Creates sub-canvas without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    Canvas subUnchecked(
        Pixel width, Pixel height,
        Pixel offset_x, Pixel offset_y
    ) {
        return Canvas{
            frame.subUnchecked(width, height, offset_x, offset_y),
            *current_font,
            foreground_color,
            background_color
        };
    }

    // Attributes

    /// @brief Get canvas width in pixels
    kf_nodiscard Pixel width() const noexcept { return frame.width; }

    /// @brief Get canvas height in pixels
    kf_nodiscard Pixel height() const noexcept { return frame.height; }

    /// @brief Get maximum valid X coordinate (rightmost pixel)
    kf_nodiscard Pixel maxX() const noexcept { return static_cast<Pixel>(width() - 1); }

    /// @brief Get maximum valid Y coordinate (bottommost pixel)
    kf_nodiscard Pixel maxY() const noexcept { return static_cast<Pixel>(height() - 1); }

    /// @brief Get horizontal center coordinate
    kf_nodiscard Pixel centerX() const noexcept { return static_cast<Pixel>(maxX() / 2); }

    /// @brief Get vertical center coordinate
    kf_nodiscard Pixel centerY() const noexcept { return static_cast<Pixel>(maxY() / 2); }

    /// @brief Get tab width based on current font (4 character widths)
    kf_nodiscard Pixel tabWidth() const noexcept { return static_cast<Pixel>(current_font->widthTotal() * 4); }

    /// @brief Get canvas width in glyphs
    kf_nodiscard u8 widthInGlyphs() const noexcept { return frame.width / current_font->widthTotal(); }

    /// @brief Get canvas height in glyphs
    kf_nodiscard u8 heightInGlyphs() const noexcept { return frame.height / current_font->heightTotal(); }

    /// @brief Get current font glyph width
    kf_nodiscard Pixel glyphWidth() const noexcept { return current_font->widthTotal(); }

    /// @brief Get current font glyph height
    kf_nodiscard Pixel glyphHeight() const noexcept { return current_font->heightTotal(); }

    // Control

    /// @brief Set current font for text rendering
    void setFont(const Font &font) { current_font = &font; }

    /// @brief Set background color for fill and text operations
    void setBackground(ColorType color) { background_color = color; }

    /// @brief Set foreground color for drawing operations
    void setForeground(ColorType color) { foreground_color = color; }

    /// @brief Swap foreground and background colors
    void swapColors() { std::swap(foreground_color, background_color); }

    /// @brief Enable/disable automatic text wrapping to next line
    void setAutoNextLine(bool enable) { auto_next_line = enable; }

    /// @brief Split canvas into weighted sub-canvases
    /// @tparam N Number of sub-canvases to create
    /// @param weights Relative weights for each sub-canvas
    /// @param horizontal True for horizontal split, false for vertical
    /// @return Array of sub-canvases with proportional sizes
    template<usize N> Array<Canvas, N> split(Array<usize, N> weights, bool horizontal = true) {
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
            Pixel size = (remaining * weights[i]) / total_weight;
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
    /// @param x X coordinate
    /// @param y Y coordinate
    void dot(Pixel x, Pixel y) const noexcept {
        frame.setPixel(x, y, foreground_color);
    }

    /// @brief Draw static image at specified position
    /// @tparam W Image width
    /// @tparam H Image height
    /// @param x Left position
    /// @param y Top position
    /// @param image Image to draw
    template<Pixel W, Pixel H> void image(Pixel x, Pixel y, const StaticImage<F, W, H> &image) noexcept {
        traits::copy(
            image.buffer, image.width(), image.height(),
            frame.buffer, frame.stride, frame.width, frame.height,
            x, y);
    }

    /// @brief Draw line (x0, y0), (x1, y1) between two points
    void line(Pixel x0, Pixel y0, Pixel x1, Pixel y1) const noexcept {
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

        const auto dx = static_cast<Pixel>(std::abs(x1 - x0));
        const auto dy = static_cast<Pixel>(-std::abs(y1 - y0));
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
                x0 = static_cast<Pixel>(x0 + sx);
            }
            if (double_error <= dx) {
                if (y0 == y1) { break; }
                error += dx;
                y0 = static_cast<Pixel>(y0 + sy);
            }
        }
    }

    /// @brief Draw rectangle (filled or outline)
    void rect(Pixel x0, Pixel y0, Pixel x1, Pixel y1, bool fill) noexcept {
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
    void circle(Pixel cx, Pixel cy, Pixel r, bool fill) noexcept {
        if (r < 0) { return; }

        if (fill) {
            // Fill circle
            const auto r_squared = r * r;
            for (auto y = -r; y <= r; y += 1) {
                const int y_squared = y * y;
                const auto width = static_cast<int>(std::sqrt(r_squared - y_squared));

                // todo lineH
                for (auto x = -width; x <= width; x += 1) {
                    frame.setPixel(static_cast<Pixel>(cx + x), static_cast<Pixel>(cy + y), foreground_color);
                }
            }
        } else {
            // Outline circle - modified Bresenham algorithm
            Pixel x = r;
            Pixel y = 0;
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
    ///   \x82 - Set cursor at center x
    ///   \x83 - Set custom foreground color (ANSI color index)
    ///   \x84 - Set custom background color (ANSI color index)
    ///   \n - New line
    ///   \t - Tab (4 character widths)
    void text(Pixel start_x, Pixel start_y, const char *text) noexcept {
        Pixel cursor_x = start_x;
        Pixel cursor_y = start_y;
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
                    current_foreground_color = Palette::getAnsiColor(static_cast<typename Palette::Ansi>(*text));
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
                    current_background_color = Palette::getAnsiColor(static_cast<typename Palette::Ansi>(*text));
                    continue;
                }

                case '\n': {
                    clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                    cursor_x = start_x;
                    cursor_y = static_cast<Pixel>(cursor_y + font_total_height);
                    continue;
                }

                case '\t': {
                    const auto tab_width = tabWidth();
                    const auto new_x = static_cast<Pixel>(((cursor_x / tab_width) + 1) * tab_width);
                    clearLineSegment(cursor_x, cursor_y, new_x, current_background_color);
                    cursor_x = new_x;
                    continue;
                }
            }

            if (cursor_x > static_cast<Pixel>(width() - font_width)) {
                clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                if (auto_next_line) {
                    cursor_x = 0;
                    cursor_y = static_cast<Pixel>(cursor_y + font_total_height);
                } else {
                    return;
                }
            }

            if (cursor_y > static_cast<Pixel>(height() - font_height)) { return; }

            drawGlyph(cursor_x, cursor_y, current_font->getGlyph(*text), current_foreground_color, current_background_color);

            cursor_x = static_cast<Pixel>(cursor_x + font_width);
            if (cursor_x < width()) {
                drawLineVertical(
                    cursor_x,
                    cursor_y,
                    static_cast<Pixel>(cursor_y + font_height),
                    current_background_color);
            }
            cursor_x = static_cast<Pixel>(cursor_x + 1);
        }
    }

private:
    // Drawing API backend

    /// @brief Clear rectangular line segment with background color
    void clearLineSegment(Pixel cursor_x, Pixel cursor_y, Pixel end_x, ColorType color) noexcept {
        if (cursor_x < end_x) {
            frame.fill(
                cursor_x, cursor_y,
                end_x, current_font->heightTotal() + cursor_y,
                color
            );
        }
    }

    /// @brief Draw horizontal line (optimized)
    void drawLineHorizontal(Pixel x0, Pixel y, Pixel x1, ColorType color) const noexcept {
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        frame.fill(x0, y, x1, y, color);
    }

    /// @brief Draw vertical line (optimized)
    void drawLineVertical(Pixel x, Pixel y0, Pixel y1, ColorType color) const noexcept {
        if (y0 > y1) { std::swap(y0, y1); }
        frame.fill(x, y0, x, y1, color);
    }

    /// @brief Draw 8 symmetric points for circle outline
    void drawCirclePoints(Pixel cx, Pixel cy, Pixel dx, Pixel dy, ColorType color) const noexcept {
        frame.setPixel(static_cast<Pixel>(cx + dx), static_cast<Pixel>(cy + dy), color);
        frame.setPixel(static_cast<Pixel>(cx + dy), static_cast<Pixel>(cy + dx), color);
        frame.setPixel(static_cast<Pixel>(cx - dy), static_cast<Pixel>(cy + dx), color);
        frame.setPixel(static_cast<Pixel>(cx - dx), static_cast<Pixel>(cy + dy), color);
        frame.setPixel(static_cast<Pixel>(cx - dx), static_cast<Pixel>(cy - dy), color);
        frame.setPixel(static_cast<Pixel>(cx - dy), static_cast<Pixel>(cy - dx), color);
        frame.setPixel(static_cast<Pixel>(cx + dy), static_cast<Pixel>(cy - dx), color);
        frame.setPixel(static_cast<Pixel>(cx + dx), static_cast<Pixel>(cy - dy), color);
    }

    /// @brief Draw font glyph at specified position
    /// @param x Left position
    /// @param y Top position
    /// @param glyph Pointer to glyph bitmap data
    /// @param color_on Color for "on" pixels
    void drawGlyph(Pixel x, Pixel y, const u8 *glyph, ColorType color_on, ColorType color_off) noexcept {
        if (nullptr == glyph) {
            // Draw box for unknown character
            const auto x1 = static_cast<Pixel>(x + current_font->glyph_width - 1);
            const auto y1 = static_cast<Pixel>(y + current_font->glyph_height - 1);

            drawLineHorizontal(x, y, x1, color_on);
            drawLineHorizontal(x, y1, x1, color_on);
            drawLineVertical(x, y, y1, color_on);
            drawLineVertical(x1, y, y1, color_on);
            return;
        }

        const u8 font_width = current_font->glyph_width;
        const u8 font_height = current_font->glyph_height;

        for (u8 col = 0; col < font_width; ++col) {
            const auto pixel_x = static_cast<Pixel>(x + col);
            const u8 glyph_byte = glyph[col];

            for (u8 row = 0; row < font_height; row += 1) {
                const auto color = (glyph_byte >> row) & 1 ? color_on : color_off;
                frame.setPixel(pixel_x, static_cast<Pixel>(y + row), color);
            }

            frame.setPixel(pixel_x, static_cast<Pixel>(y + font_height), color_off);
        }
    }
};

}// namespace kf::gfx
