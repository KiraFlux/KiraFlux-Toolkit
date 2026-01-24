// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/algorithm.hpp"
#include "kf/core/attributes.hpp"
#include "kf/core/pixel_traits.hpp"
#include "kf/math/units.hpp"


namespace kf::gfx {

/// @brief Dynamic display region with runtime dimensions
/// @tparam Format Pixel format for the image data
template<PixelFormat Format> struct DynamicImage final {

public:
    /// @brief Possible errors when creating FrameView
    enum class Error : u8 {
        /// @brief Buffer pointer is null
        BufferNotInit,

        /// @brief Region dimensions are less than 1 pixel
        SizeTooSmall,

        /// @brief Sub-region exceeds parent bounds
        SizeTooLarge,

        /// @brief Offset falls outside parent region
        OffsetOutOfBounds,
    };

private:
    using Traits = pixel_traits<Format>;

public:
    using BufferType = typename Traits::BufferType;///< Raw buffer element type
    using ColorType = typename Traits::ColorType;  ///< Pixel color representation

    /// @brief Pointer to display buffer memory
    BufferType *buffer;

    /// @brief Row stride (full display width)
    Pixel stride;

    /// @brief Absolute X offset from buffer origin
    Pixel offset_x;

    /// @brief Absolute Y offset from buffer origin
    Pixel offset_y;

    /// @brief Region width in pixels
    Pixel width;

    /// @brief Region height in pixels
    Pixel height;

    /// @brief Creates FrameView with validation
    kf_nodiscard static Result<DynamicImage, Error> create(
        BufferType *buffer, Pixel stride,
        Pixel width, Pixel height,
        Pixel offset_x, Pixel offset_y
    ) noexcept {
        if (nullptr == buffer) {
            return Error::BufferNotInit;
        }

        if (width < 1 or height < 1) {
            return Error::SizeTooSmall;
        }

        return DynamicImage(buffer, stride, width, height, offset_x, offset_y);
    }

    /// @brief Default constructor - invalid view
    DynamicImage() :
        buffer{nullptr}, stride{0}, offset_x{0}, offset_y{0}, width{0}, height{0} {};

    /// @brief Creates FrameView without validation
    /// @warning Caller must ensure parameters are valid
    explicit DynamicImage(
        BufferType *buffer, Pixel stride,
        Pixel width, Pixel height,
        Pixel offset_x, Pixel offset_y
    ) noexcept:
        buffer{buffer},
        stride{stride},
        offset_x{offset_x},
        offset_y{offset_y},
        width{width},
        height{height} {}

    /// @brief Creates validated sub-region
    /// @return Sub-view or error if out of bounds
    kf_nodiscard Result<DynamicImage, Error> sub(
        Pixel sub_width, Pixel sub_height,
        Pixel sub_offset_x, Pixel sub_offset_y
    ) const noexcept {
        if (sub_offset_x >= width or sub_offset_y >= height) {
            return Error::OffsetOutOfBounds;
        }

        if (sub_width > width - sub_offset_x or sub_height > height - sub_offset_y) {
            return Error::SizeTooLarge;
        }

        const auto new_x = static_cast<Pixel>(offset_x + sub_offset_x);
        const auto new_y = static_cast<Pixel>(offset_y + sub_offset_y);

        return create(buffer, stride, sub_width, sub_height, new_x, new_y);
    }

    /// @brief Creates sub-region without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    kf_nodiscard DynamicImage subUnchecked(
        Pixel sub_width, Pixel sub_height,
        Pixel sub_offset_x, Pixel sub_offset_y
    ) {
        return DynamicImage{
            buffer, stride, sub_width, sub_height,
            static_cast<Pixel>(offset_x + sub_offset_x),
            static_cast<Pixel>(offset_y + sub_offset_y)
        };
    }

    /// @brief Checks if X coordinate is within view bounds
    /// @param x Relative X coordinate
    /// @return True if coordinate is valid
    kf_nodiscard inline bool isInsideX(Pixel x) const { return x >= 0 and x < width; }

    /// @brief Checks if Y coordinate is within view bounds
    /// @param y Relative Y coordinate
    /// @return True if coordinate is valid
    kf_nodiscard inline bool isInsideY(Pixel y) const { return y >= 0 and y < height; }

    /// @brief Checks if view references valid buffer
    /// @return True if buffer pointer is not null
    kf_nodiscard bool isValid() const { return nullptr != buffer; }

    /// @brief Sets single pixel color
    /// @param x Relative X coordinate
    /// @param y Relative Y coordinate
    /// @param color Pixel color value
    inline void setPixel(Pixel x, Pixel y, ColorType color) const noexcept {
        Traits::setPixel(buffer, stride, toAbsoluteX(x), toAbsoluteY(y), color);
    }

    /// @brief Fills entire region with solid color
    /// @param color Fill color value
    inline void fill(ColorType color) const noexcept {
        Traits::fill(buffer, stride, offset_x, offset_y, width, height, color);
    }

    /// @brief Fills rect region with solid color
    /// @param color Fill color value
    void fill(
        Pixel x0, Pixel y0,
        Pixel x1, Pixel y1,
        ColorType color
    ) const noexcept {
        Traits::fill(
            buffer,
            stride,
            static_cast<Pixel>(offset_x + x0),
            static_cast<Pixel>(offset_y + y0),
            static_cast<Pixel>(x1 - x0 + 1),
            static_cast<Pixel>(y1 - y0 + 1),
            color
        );
    }

private:
    /// @brief Converts relative X to absolute buffer coordinate
    kf_nodiscard inline Pixel toAbsoluteX(Pixel x) const noexcept {
        return static_cast<Pixel>(offset_x + x);
    }

    /// @brief Converts relative Y to absolute buffer coordinate
    kf_nodiscard inline Pixel toAbsoluteY(Pixel y) const noexcept {
        return static_cast<Pixel>(offset_y + y);
    }
};
}// namespace kf::gfx
