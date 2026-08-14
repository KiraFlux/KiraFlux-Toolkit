// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/BinaryReadable.hpp
/// @brief   Adds readByte/readBuffer/readPacket for binary streams.

#pragma once

#include "kf/Bytes.hpp"
#include "kf/BytesView.hpp"
#include "kf/Result.hpp"
#include "kf/core.hpp"

namespace kf::mixin {

struct BinaryReadableTag {};

/// @brief CRTP base class for readable byte streams
/// @tparam Impl      Derived class
/// @tparam ErrorImpl Error type used by the implementation
/// @note Derived classes must implement:
///
///       - `auto readBufferImpl(Bytes buffer) noexcept -> Result<BytesView, Error>`
///         Read up to buffer.size() bytes into the given buffer. Returns a slice of actually read data
///
///       - `template<typename T> auto readPacketImpl() noexcept -> Result<T, Error>`
///         Read a trivially copyable object of type T (size fixed at compile time)
template<typename Impl, typename ErrorImpl> struct BinaryReadable : BinaryReadableTag {

    /// @brief Read single byte
    /// @return Byte value or error
    [[nodiscard]] auto readByte() noexcept -> Result<u8, ErrorImpl> {
        return this->readPacket<u8>();
    }

    /// @brief Read arbitrary number of bytes into buffer
    /// @param buffer Destination buffer
    /// @return Slice containing actually read data, or error
    [[nodiscard]] auto readBuffer(Bytes buffer) noexcept -> Result<BytesView, ErrorImpl> {
        return static_cast<Impl *>(this)->readBufferImpl(buffer);
    }

    /// @brief Read fixed‑size buffer
    /// @tparam T Type of packet
    /// @return Packet value or error
    template<trivial T> [[nodiscard]] auto readPacket() noexcept -> Result<T, ErrorImpl> {
        return static_cast<Impl *>(this)->template readPacketImpl<T>();
    }
};

}// namespace kf::mixin

#define KF_IMPL_BINARY_READABLE(...) friend struct kf::mixin::BinaryReadable<__VA_ARGS__>
