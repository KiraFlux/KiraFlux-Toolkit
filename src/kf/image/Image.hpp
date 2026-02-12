// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/attributes.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Slice.hpp"

namespace kf::image {

/// @brief Image
/// @tparam Impl Image implementation
/// @tparam F kf::pixel::PixelFormat type
template<typename Impl, typename F> struct Image {
    using PixelFormat = F;
    using BufferType = typename F::BufferType;
    using ColorType = typename F::ColorType;

    // Abstract properties

    /// @brief Get current width in pixels (may differ from physical width due to orientation)
    kf_nodiscard Pixel width() const noexcept { return c_impl().getWidthImpl(); }

    /// @brief Get current height in pixels (may differ from physical width due to orientation)
    kf_nodiscard Pixel height() const noexcept { return c_impl().getHeightImpl(); }

    /// @brief Get current full width in pixels (may differ from physical width due to orientation)
    kf_nodiscard Pixel stride() const noexcept { return c_impl().getStrideImpl(); }

    /// @brief Get writable frame buffer
    kf_nodiscard Slice<BufferType> buffer() noexcept { return impl().getBufferImpl(); }

    /// @brief Get readonly frame buffer
    kf_nodiscard Slice<const BufferType> buffer() const noexcept { return c_impl().getBufferImpl(); }

    // properties

    /// @brief Get maximum valid X coordinate
    kf_nodiscard u8 maxX() const noexcept { return width() - 1; }

    /// @brief Get maximum valid Y coordinate
    kf_nodiscard u8 maxY() const noexcept { return height() - 1; }

    // CRTP
    friend Impl;

private:
    inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }

    inline const Impl &c_impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::image