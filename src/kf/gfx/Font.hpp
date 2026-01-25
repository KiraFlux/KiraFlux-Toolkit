// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/core/attributes.hpp"

namespace kf::gfx {

/// @brief Monospaced font with glyph height up to 8 pixels
/// @details Represents a bitmap font for monochrome displays.
/// Each glyph is stored as a horizontal bitmask where each byte represents
/// a vertical column of pixels. Glyphs are stored consecutively in memory.
/// The font data must be stored in program memory (Flash) for embedded systems.
struct Font final {
    /// @brief First character code in the font (inclusive)
    static constexpr char start_char = 32;///< ASCII space character (0x20)

    /// @brief Last character code NOT included in the font (exclusive)
    static constexpr char end_char = 127;///< ASCII DEL character (0x7F), not included

    /// @brief Pointer to font glyph data
    /// @details Array of glyph bitmaps stored consecutively in memory.
    /// Each glyph consists of `glyph_width` bytes, where each byte represents
    /// a vertical column of pixels. Within each byte:
    /// - Bit 0: top pixel (LSB)
    /// - Bit 1-6: middle pixels (for 7px high fonts)
    /// - Bit 7: bottom pixel (for 8px high fonts) or unused
    /// Glyphs are stored in ASCII order starting from `start_char`.
    /// Example: For 5×7 font (5 columns, 7 rows):
    ///   Each glyph = 5 bytes, each byte = 1 vertical column (8 bits, but only 7 used)
    /// @warning Must point to valid memory (Flash/PROGMEM for embedded systems)
    const u8 *data;

    /// @brief Width of each glyph in pixels (1-255)
    /// @details Horizontal size of every character in the font.
    /// This is a fixed value for monospaced fonts.
    /// Example: 5 for 5×7 font means 5 pixels wide, 5 bytes per glyph
    const u8 glyph_width;

    /// @brief Height of each glyph in pixels (1-8)
    /// @details Vertical size of every character in the font.
    /// Must be between 1 and 8 inclusive for 1-byte per column representation.
    /// Example: 7 for 5×7 font means 7 pixels high, using bits 0-6 of each byte
    const u8 glyph_height;

    /// @brief Get an instance of empty/blank font
    /// @return Reference to singleton empty font instance
    /// @details Returns a font with null data pointer that can be used as
    /// a placeholder or default. When used for rendering, all characters
    /// will appear as spaces (no visible glyphs).
    /// @note The blank font has nominal dimensions of 3×5 pixels
    static const Font &blank() noexcept {
        static Font instance{
            nullptr,
            3,
            5,
        };
        return instance;
    }

    /// @brief Get total width including inter-character spacing
    /// @return Total advance width in pixels (glyph_width + 1)
    /// @details Includes 1 pixel of spacing between characters.
    /// Use this for cursor positioning and text layout calculations.
    kf_nodiscard inline u8 widthTotal() const noexcept { return glyph_width + 1; }

    /// @brief Get total height including inter-line spacing
    /// @return Total line height in pixels (glyph_height + 1)
    /// @details Includes 1 pixel of spacing between lines.
    /// Use this for multi-line text layout calculations.
    kf_nodiscard inline u8 heightTotal() const noexcept { return glyph_height + 1; }

    /// @brief Get pointer to glyph data for a character
    /// @param c Character code (ASCII)
    /// @return Pointer to glyph bitmap data, or nullptr if:
    ///         - Font data is null
    ///         - Character is outside font range [start_char, end_char)
    /// @details The returned pointer points to the beginning of the glyph's
    /// bitmap data in the font data array. The glyph data consists of
    /// `glyph_width` consecutive bytes, each representing a vertical column.
    /// @note ASCII code 127 (DEL) is excluded from the font
    kf_nodiscard const u8 *getGlyph(char c) const noexcept {
        if (nullptr == data or c < start_char or c >= end_char) {
            return nullptr;
        }

        return data + (static_cast<usize>(c - start_char) * glyph_width);
    }
};

/// @brief System fonts collection
/// @details Predefined fonts for common display resolutions and applications.
/// All fonts are stored in program memory (Flash) with external linkage.
namespace fonts {

/// @brief GyverOLED 5×7 English font
/// @details Optimized for 128×64 OLED displays. Contains ASCII characters
/// from space (0x20) to tilde (0x7E). Each glyph is 5 pixels wide and 7 pixels high.
/// Font data format: 5 bytes per glyph, each byte = 1 vertical column (8 bits, bits 0-6 used).
/// Total glyphs: 126 - 32 + 1 = 95 characters.
/// @note This font does not include Cyrillic or other non-ASCII characters
extern const Font gyver_5x7_en;

}// namespace fonts
}// namespace kf::gfx