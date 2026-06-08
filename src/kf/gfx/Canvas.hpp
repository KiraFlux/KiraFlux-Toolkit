// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Option.hpp"
#include "kf/Result.hpp"
#include "kf/gfx/Font.hpp"
#include "kf/gfx/Palette.hpp"
#include "kf/image/DynamicImage.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/pixel/Pixel.hpp"

namespace kf::gfx {

/// @brief Drawing context with graphics primitives and text rendering
/// @tparam P Pixel format for canvas operations
template<typename P> struct Canvas {
    KF_CHECK_IMPL(P, ::kf::pixel::PixelTag);

    using PixelImpl = P;
    using ColorType = typename PixelImpl::ColorType;
    using PaletteType = Palette<PixelImpl>;

    struct State {
        Option<const Font &> active_font;///< Currently selected font
        ColorType foreground_color;      ///< Drawing color
        ColorType background_color;      ///< Background/fill color
        bool auto_next_line;             ///< Automatically wrap text to next line

        [[nodiscard]] static constexpr State defaults() noexcept {
            return State{
                .active_font = none,
                .foreground_color = ColorType{},
                .background_color = ColorType{},
                .auto_next_line = false,
            };
        }
    };

    explicit constexpr Canvas(const image::DynamicImage<P> &frame, State state = State::defaults()) noexcept : _frame{frame}, _state{state} {}

    /// @brief Creates validated sub-canvas within current bounds
    [[nodiscard]] auto sub(
        math::Pixels width, math::Pixels height,
        math::Pixels offset_x, math::Pixels offset_y) noexcept -> Result<Canvas, typename image::DynamicImage<PixelImpl>::Error> {
        return _frame.sub(width, height, offset_x, offset_y).map([this](auto frame) {
            return Canvas{frame, _state};
        });
    }

    /// @brief Creates sub-canvas without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    [[nodiscard]] constexpr Canvas subUnchecked(
        math::Pixels width, math::Pixels height,
        math::Pixels offset_x, math::Pixels offset_y) noexcept {
        return Canvas{
            _frame.subUnchecked(width, height, offset_x, offset_y),
            _state,
        };
    }

    // Properties: Dimentions

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
    [[nodiscard]] math::Pixels tabWidth() const noexcept { return static_cast<math::Pixels>(font().widthTotal() * 4); }

    /// @brief Get canvas width in glyphs
    [[nodiscard]] usize widthInGlyphs() const noexcept { return width() / font().widthTotal(); }

    /// @brief Get canvas height in glyphs
    [[nodiscard]] usize heightInGlyphs() const noexcept { return height() / font().heightTotal(); }

    // Properties: Colors

    /// @brief Get current Foreground color
    [[nodiscard]] ColorType foreground() const noexcept { return _state.foreground_color; }

    /// @brief Set foreground color for drawing operations
    void foreground(ColorType color) noexcept { _state.foreground_color = color; }

    /// @brief Get current Background color
    [[nodiscard]] ColorType background() const noexcept { return _state.background_color; }

    /// @brief Set background color for fill and text operations
    void background(ColorType color) noexcept { _state.background_color = color; }

    // Properties: Font

    /// @brief Get current text font
    [[nodiscard]] const Font &font() const noexcept { return _state.active_font.unwrapOr(Font::blank()); }

    /// @brief Set current text font
    void font(const Font &new_font) noexcept { _state.active_font = someRef(new_font); }

    // Control

    /// @brief Enable/disable automatic text wrapping to next line
    void autoNextLine(bool enable) noexcept { _state.auto_next_line = enable; }

    /// @brief Swap foreground and background colors
    void swapColors() noexcept { std::swap(_state.foreground_color, _state.background_color); }

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

        alignas(Canvas) u8 buffer[sizeof(Canvas) * N];

        auto &result = *reinterpret_cast<memory::Array<Canvas, N> *>(&buffer);

        for (auto i = 0u; i < N; i += 1) {
            auto size = (remaining * weights[i]) / total_weight;
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

        return {result};
    }

    // Drawing API

    /// @brief Fill entire canvas with background color
    void fill() const noexcept {
        _frame.fill(_state.background_color);
    }

    /// @brief Draw single pixel at specified coordinates
    void dot(math::Pixels x, math::Pixels y) const noexcept {
        _frame.setPixel(x, y, _state.foreground_color);
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
                _frame.setPixel(x0, y0, _state.foreground_color);
            } else {
                drawLineVertical(x0, y0, y1, _state.foreground_color);
            }
            return;
        }

        if (y0 == y1) {
            drawLineHorizontal(x0, y0, x1, _state.foreground_color);
            return;
        }

        const auto dx = std::abs(x1 - x0);
        const auto dy = -std::abs(y1 - y0);
        const auto sx = (x0 < x1) ? 1 : -1;
        const auto sy = (y0 < y1) ? 1 : -1;
        auto error = dx + dy;

        while (true) {
            _frame.setPixel(x0, y0, _state.foreground_color);
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
            _frame.fill(x0, y0, x1, y1, _state.foreground_color);
        } else {
            // Outline
            drawLineHorizontal(x0, y0, x1, _state.foreground_color);
            drawLineHorizontal(x0, y1, x1, _state.foreground_color);
            y0 += 1;
            drawLineVertical(x0, y0, y1, _state.foreground_color);
            drawLineVertical(x1, y0, y1, _state.foreground_color);
        }
    }

    /// @brief Draw circle (filled or outline)
    void circle(math::Pixels center_x, math::Pixels center_y, math::Pixels radius, bool fill) noexcept {
        if (radius < 0) { return; }

        if (fill) {
            // Fill circle
            const auto r_squared = radius * radius;
            for (auto y = -radius; y <= radius; y += 1) {
                const int y_squared = y * y;
                const auto width = static_cast<int>(std::sqrt(r_squared - y_squared));

                // todo lineH
                for (auto x = -width; x <= width; x += 1) {
                    _frame.setPixel(static_cast<math::Pixels>(center_x + x), static_cast<math::Pixels>(center_y + y), _state.foreground_color);
                }
            }
        } else {
            // Outline circle - modified Bresenham algorithm
            math::Pixels x = radius;
            math::Pixels y = 0;
            int err = 0;

            while (x >= y) {
                // Draw 8 symmetric points
                drawCirclePoints(center_x, center_y, x, y, _state.foreground_color);

                // Avoid drawing duplicate points when x == y
                if (x != y) {
                    drawCirclePoints(center_x, center_y, y, x, _state.foreground_color);
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

    /// @brief Draw glyph at specified position
    void glyph(math::Pixels x, math::Pixels y, char c) noexcept {
        drawGlyph(x, y, font().getGlyph(c), _state.foreground_color, _state.background_color);
    }

    /// @brief Draw text at specified position
    /// @note Supports formatting codes:
    ///
    ///   \x80 - Normal color mode (text - fg, bg)
    ///
    ///   \x81 - Invert color mode (bg, text - fg)
    ///
    ///   \x82 - swap bg <-> fg colors
    ///
    ///   \xF* - Set custom foreground color (ANSI color index)
    ///
    ///   \xB* - Set custom background color (ANSI color index)
    ///
    ///   \n - New line
    ///
    ///   \t - Tab (4 character widths)
    void text(math::Pixels start_x, math::Pixels start_y, memory::StringView text) noexcept {
        const auto font_width = font().glyph_width;
        const auto font_height = font().glyph_height;
        const auto font_total_height = font().heightTotal();

        auto cursor_x = start_x;
        auto cursor_y = start_y;

        auto current_foreground_color = _state.foreground_color;
        auto current_background_color = _state.background_color;

        for (char c: text) {
            switch (c) {
                case '\x80': {
                    current_foreground_color = _state.foreground_color;
                    current_background_color = _state.background_color;
                    continue;
                }

                case '\x81': {
                    current_foreground_color = _state.background_color;
                    current_background_color = _state.foreground_color;
                    continue;
                }

                case '\x82': {
                    std::swap(current_background_color, current_foreground_color);
                    continue;
                }

                case '\xF0' ... '\xFF': {
                    current_foreground_color = PaletteType::ansiColor(static_cast<u8>(c));
                    continue;
                }

                case '\xB0' ... '\xBF': {
                    current_background_color = PaletteType::ansiColor(static_cast<u8>(c));
                    continue;
                }

                case '\n': {
                    clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                    cursor_x = start_x;
                    cursor_y += font_total_height;
                    continue;
                }

                case '\t': {
                    const auto tab_width = tabWidth();
                    const auto new_x = ((cursor_x / tab_width) + 1) * tab_width;
                    clearLineSegment(cursor_x, cursor_y, new_x, current_background_color);
                    cursor_x = new_x;
                    continue;
                }
            }

            if (cursor_x > (width() - font_width)) {
                clearLineSegment(cursor_x, cursor_y, maxX(), current_background_color);
                if (_state.auto_next_line) {
                    cursor_x = start_x;
                    cursor_y += font_total_height;
                } else {
                    return;
                }
            }

            if (cursor_y > (height() - font_height)) { return; }

            drawGlyph(cursor_x, cursor_y, font().getGlyph(c), current_foreground_color, current_background_color);

            cursor_x += font_width;
            if (cursor_x < width()) {
                drawLineVertical(cursor_x, cursor_y, cursor_y + font_height, current_background_color);
            }

            cursor_x += 1;
        }
    }

private:
    image::DynamicImage<P> _frame;///< Target drawing surface
    State _state;

    // Drawing API backend

    /// @brief Clear rectangular line segment with background color
    void clearLineSegment(math::Pixels cursor_x, math::Pixels cursor_y, math::Pixels end_x, ColorType color) noexcept {
        if (cursor_x < end_x) {
            _frame.fill(cursor_x, cursor_y, end_x, font().heightTotal() + cursor_y, color);
        }
    }

    /// @brief Draw horizontal line (optimized)
    void drawLineHorizontal(math::Pixels x0, math::Pixels y, math::Pixels x1, ColorType color) const noexcept {
        if (x0 > x1) { std::swap(x0, x1); }
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
            const auto x1 = static_cast<math::Pixels>(x + font().glyph_width - 1);
            const auto y1 = static_cast<math::Pixels>(y + font().glyph_height - 1);

            drawLineHorizontal(x, y, x1, color_on);
            drawLineHorizontal(x, y1, x1, color_on);
            drawLineVertical(x, y, y1, color_on);
            drawLineVertical(x1, y, y1, color_on);
            return;
        }

        const auto font_width = font().glyph_width;
        const auto font_height = font().glyph_height;

        for (auto col = 0u; col < font_width; col += 1) {
            const auto pixel_x = static_cast<math::Pixels>(x + col);

            for (auto row = 0u; row < font_height; row += 1) {
                const auto color = (glyph[col] >> row) & 1 ? color_on : color_off;
                _frame.setPixel(pixel_x, static_cast<math::Pixels>(y + row), color);
            }

            _frame.setPixel(pixel_x, static_cast<math::Pixels>(y + font_height), color_off);
        }
    }
};

}// namespace kf::gfx