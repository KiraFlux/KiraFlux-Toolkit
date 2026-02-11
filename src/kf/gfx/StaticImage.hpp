// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/attributes.hpp"
#include "kf/math/units.hpp"

namespace kf::gfx {

/// @brief Predefined bitmap image with compile-time dimensions
/// @tparam F Pixel format for the image
/// @tparam W Image width in pixels (compile-time constant)
/// @tparam H Image height in pixels (compile-time constant)
/// @details Represents a static image with fixed dimensions stored in memory.
/// The image buffer is embedded directly in the object and cannot be resized.
/// Useful for storing icons, logos, and other predefined graphics.
template<typename F, Pixel W, Pixel H> struct StaticImage final {
    using PixelFormat = F;
    using BufferType = typename F::BufferType;

    /// @brief Get the image width in pixels (compile-time constant)
    kf_nodiscard inline constexpr Pixel width() const noexcept { return W; }

    /// @brief Get the image height in pixels (compile-time constant)
    kf_nodiscard inline constexpr Pixel height() const noexcept { return H; }

    /// @brief Get the image size
    kf_nodiscard inline constexpr usize size() const noexcept { return PixelFormat::template buffer_size<W, H>; }

    /// @brief Raw image buffer data
    /// @details Contains the pixel data for the entire image.
    /// The buffer size is calculated at compile time based on:
    /// - Pixel format (bits per pixel)
    /// - Image dimensions (W x H)
    /// - Platform alignment requirements
    /// The buffer is stored as an array of the underlying pixel type
    BufferType buffer[PixelFormat::template buffer_size<W, H>];

    /// @brief Default constructor is deleted
    /// @note StaticImage objects must be initialized with image data.
    /// Use initializer lists or designated initializers to populate the buffer.
    StaticImage() = delete;
};

}// namespace kf::gfx