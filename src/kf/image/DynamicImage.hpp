// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/Result.hpp"
#include "kf/image/Image.hpp"
#include "kf/math/units.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/pixel/PixelTag.hpp"

namespace kf::image {

/// @brief Dynamic display region with runtime dimensions
/// @tparam P Pixel implementation
template<typename P> struct DynamicImage final : Image<DynamicImage<P>, P> {
    kf_crtp_check(P, pixel::PixelTag);

    using PixelImpl = P;
    using BufferType = typename P::BufferType;
    using ColorType = typename P::ColorType;

    /// @brief Possible errors when creating FrameView
    enum class Error : u8 {
        BufferNotInit,    ///< Buffer pointer is null
        SizeTooSmall,     ///< Region dimensions are less than 1 pixel
        SizeTooLarge,     ///< Sub-region exceeds parent bounds
        OffsetOutOfBounds,///< Offset falls outside parent region
    };

private:
    Slice<BufferType> _buffer;///< display buffer memory view
    Pixels _width, _height;   ///< Region size in pixels
    Pixels _stride;           ///< Row stride (full display width)
    Pixels offset_x, offset_y;///< Absolute offset from buffer origin

public:
    /// @brief Creates FrameView with validation
    [[nodiscard]] static Result<DynamicImage, Error> create(
        Slice<BufferType> buffer, Pixels stride,
        Pixels width, Pixels height,
        Pixels offset_x, Pixels offset_y) noexcept {
        if (nullptr == buffer.data()) {
            return Error::BufferNotInit;
        }

        if (width < 1 or height < 1) {
            return Error::SizeTooSmall;
        }

        return DynamicImage(buffer, stride, width, height, offset_x, offset_y);
    }

    /// @brief Default constructor - invalid view
    DynamicImage() noexcept :
        _buffer{}, _stride{0}, offset_x{0}, offset_y{0}, _width{0}, _height{0} {};

    /// @brief Creates FrameView without validation
    /// @warning Caller must ensure parameters are valid
    explicit DynamicImage(
        Slice<BufferType> buffer, Pixels stride,
        Pixels width, Pixels height,
        Pixels offset_x, Pixels offset_y) noexcept :
        _buffer{buffer}, _stride{stride}, _width{width}, _height{height}, offset_x{offset_x}, offset_y{offset_y} {}

    template<typename I> explicit DynamicImage(
        I &image) noexcept :
        _buffer{image.buffer()}, _stride{image.stride()}, _width{image.width()}, _height{image.height()}, offset_x{0}, offset_y{0} {}

    /// @brief Creates validated sub-region
    /// @return Sub-view or error if out of bounds
    [[nodiscard]] Result<DynamicImage, Error> sub(
        Pixels sub_width, Pixels sub_height,
        Pixels sub_offset_x, Pixels sub_offset_y) const noexcept {
        if (sub_offset_x >= _width or sub_offset_y >= _height) {
            return Error::OffsetOutOfBounds;
        }

        if (sub_width > _width - sub_offset_x or sub_height > _height - sub_offset_y) {
            return Error::SizeTooLarge;
        }
        return create(
            _buffer, _stride,
            sub_width, sub_height,
            static_cast<Pixels>(offset_x + sub_offset_x), static_cast<Pixels>(offset_y + sub_offset_y));
    }

    /// @brief Creates sub-region without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    [[nodiscard]] DynamicImage subUnchecked(
        Pixels sub_width, Pixels sub_height,
        Pixels sub_offset_x, Pixels sub_offset_y) noexcept {
        return DynamicImage{
            _buffer, _stride, sub_width, sub_height,
            static_cast<Pixels>(offset_x + sub_offset_x),
            static_cast<Pixels>(offset_y + sub_offset_y)};
    }

    /// @brief Checks if view references valid buffer
    [[nodiscard]] bool isValid() const noexcept { return nullptr != _buffer.data(); }

    /// @brief Checks if X coordinate is within view bounds
    [[nodiscard]] bool isInsideX(Pixels x_relative) const noexcept { return x_relative >= 0 and x_relative < _width; }

    /// @brief Checks if Y coordinate is within view bounds
    [[nodiscard]] bool isInsideY(Pixels y_relative) const noexcept { return y_relative >= 0 and y_relative < _height; }

    /// @brief Converts relative X to absolute buffer coordinate
    [[nodiscard]] Pixels toAbsoluteX(Pixels x) const noexcept { return static_cast<Pixels>(offset_x + x); }

    /// @brief Converts relative Y to absolute buffer coordinate
    [[nodiscard]] Pixels toAbsoluteY(Pixels y) const noexcept { return static_cast<Pixels>(offset_y + y); }

    /// @brief Sets single pixel color
    void setPixel(Pixels x_relative, Pixels y_relative, ColorType color) const noexcept {
        PixelImpl::setPixel(_buffer, _stride, toAbsoluteX(x_relative), toAbsoluteY(y_relative), color);
    }

    /// @brief Fills entire region with solid color
    void fill(ColorType color) const noexcept {
        PixelImpl::fill(_buffer, _stride, offset_x, offset_y, _width, _height, color);
    }

    /// @brief Fills rect region with solid color
    void fill(
        Pixels x0, Pixels y0,
        Pixels x1, Pixels y1,
        ColorType color) const noexcept {
        if (x0 > x1) { std::swap(x0, x1); }
        if (y0 > y1) { std::swap(y0, y1); }

        PixelImpl::fill(
            _buffer,
            _stride,
            static_cast<Pixels>(offset_x + x0),
            static_cast<Pixels>(offset_y + y0),
            static_cast<Pixels>(x1 - x0 + 1),
            static_cast<Pixels>(y1 - y0 + 1),
            color);
    }

    // CRTP
private:
    friend Image<DynamicImage<P>, P>;

    [[nodiscard]] constexpr Pixels getWidthImpl() const noexcept { return _width; }

    [[nodiscard]] constexpr Pixels getHeightImpl() const noexcept { return _height; }

    [[nodiscard]] constexpr Pixels getStrideImpl() const noexcept { return _stride; }

    [[nodiscard]] constexpr Slice<BufferType> getBufferImpl() noexcept { return _buffer; }

    [[nodiscard]] constexpr Slice<const BufferType> getBufferImpl() const noexcept { return _buffer; }
};

}// namespace kf::image
