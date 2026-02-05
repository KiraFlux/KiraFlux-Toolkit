// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/attributes.hpp"
#include "kf/Result.hpp"
#include "kf/math/units.hpp"


namespace kf { // NOLINT(*-concat-nested-namespaces)
namespace gfx {

/// @brief Dynamic display region with runtime dimensions
/// @tparam F Pixel format for the image data
template<typename F> struct DynamicImage final {
    using PixelFormat = F;
    using BufferType = typename F::BufferType;
    using ColorType = typename F::ColorType;

    /// @brief Possible errors when creating FrameView
    enum class Error : u8 {
        BufferNotInit, ///< Buffer pointer is null
        SizeTooSmall, ///< Region dimensions are less than 1 pixel
        SizeTooLarge, ///< Sub-region exceeds parent bounds
        OffsetOutOfBounds, ///< Offset falls outside parent region
    };

    BufferType *buffer; ///< Pointer to display buffer memory
    Pixel stride;             ///< Row stride (full display width)
    Pixel offset_x, offset_y; ///< Absolute offset from buffer origin
    Pixel width, height;      ///< Region size in pixels

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
    DynamicImage() noexcept:
        buffer{nullptr}, stride{0}, offset_x{0}, offset_y{0}, width{0}, height{0} {};

    /// @brief Creates FrameView without validation
    /// @warning Caller must ensure parameters are valid
    explicit DynamicImage(
        BufferType *buffer, Pixel stride,
        Pixel width, Pixel height,
        Pixel offset_x, Pixel offset_y
    ) noexcept:
        buffer{buffer}, stride{stride}, offset_x{offset_x}, offset_y{offset_y}, width{width}, height{height} {}

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
        return create(
            buffer, stride,
            sub_width, sub_height,
            static_cast<Pixel>(offset_x + sub_offset_x), static_cast<Pixel>(offset_y + sub_offset_y)
        );
    }

    /// @brief Creates sub-region without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    kf_nodiscard DynamicImage subUnchecked(
        Pixel sub_width, Pixel sub_height,
        Pixel sub_offset_x, Pixel sub_offset_y
    ) noexcept {
        return DynamicImage{
            buffer, stride, sub_width, sub_height,
            static_cast<Pixel>(offset_x + sub_offset_x),
            static_cast<Pixel>(offset_y + sub_offset_y)
        };
    }

    /// @brief Checks if view references valid buffer
    kf_nodiscard bool isValid() const noexcept { return nullptr != buffer; }

    /// @brief Checks if X coordinate is within view bounds
    kf_nodiscard bool isInsideX(Pixel x_relative) const noexcept { return x_relative >= 0 and x_relative < width; }

    /// @brief Checks if Y coordinate is within view bounds
    kf_nodiscard bool isInsideY(Pixel y_relative) const noexcept { return y_relative >= 0 and y_relative < height; }

    /// @brief Converts relative X to absolute buffer coordinate
    kf_nodiscard Pixel toAbsoluteX(Pixel x) const noexcept { return static_cast<Pixel>(offset_x + x); }

    /// @brief Converts relative Y to absolute buffer coordinate
    kf_nodiscard Pixel toAbsoluteY(Pixel y) const noexcept { return static_cast<Pixel>(offset_y + y); }

    /// @brief Sets single pixel color
    void setPixel(
        Pixel x_relative, Pixel y_relative,
        ColorType color
    ) const noexcept {
        PixelFormat::setPixel(buffer, stride, toAbsoluteX(x_relative), toAbsoluteY(y_relative), color);
    }

    /// @brief Fills entire region with solid color
    void fill(
        ColorType color
    ) const noexcept {
        PixelFormat::fill(buffer, stride, offset_x, offset_y, width, height, color);
    }

    /// @brief Fills rect region with solid color
    void fill(
        Pixel x0, Pixel y0,
        Pixel x1, Pixel y1,
        ColorType color
    ) const noexcept {
        PixelFormat::fill(
            buffer,
            stride,
            static_cast<Pixel>(offset_x + x0),
            static_cast<Pixel>(offset_y + y0),
            static_cast<Pixel>(x1 - x0 + 1),
            static_cast<Pixel>(y1 - y0 + 1),
            color
        );
    }
};

}
}
