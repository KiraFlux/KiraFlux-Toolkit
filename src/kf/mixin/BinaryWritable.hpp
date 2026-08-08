// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/BinaryWritable.hpp
/// @brief   Adds writeByte/writeBuffer/writePacket for binary streams.

#pragma once

#include "kf/BytesView.hpp"
#include "kf/concepts.hpp"
#include "kf/primitives.hpp"

namespace kf::mixin {

struct BinaryWritableTag {};

/// @brief CRTP base class for writable streams
/// @tparam Impl      Derived class
/// @tparam ErrorImpl Error type used by the implementation
/// @note Derived classes must implement:
///
///       - `ResultType writeBufferImpl(BytesView buffer) noexcept`
///         Write a contiguous buffer of bytes
///
///       - `ResultType writePacketImpl(trivial auto const &packet) noexcept`
///         Write a trivially copyable object
///
///       - `ResultType writeMixedImpl(trivial auto const &header, BytesView buffer) noexcept`
///         Write a small header followed by a buffer (e.g. command + data) in one transaction
template<typename Impl, typename ResultType> struct BinaryWritable : BinaryWritableTag {

    /// @brief Write single byte
    /// @return Result indicating success or error
    [[nodiscard]] ResultType writeByte(u8 byte) noexcept {
        return this->writePacket(byte);
    }

    /// @brief Write arbitrary data from buffer
    /// @param buffer Source data
    /// @return Result indicating success or error
    [[nodiscard]] ResultType writeBuffer(BytesView buffer) noexcept {
        return static_cast<Impl *>(this)->writeBufferImpl(buffer);
    }

    /// @brief Write fixed‑size packet
    /// @param packet Value to write
    /// @return Result indicating success or error
    [[nodiscard]] ResultType writePacket(trivial auto const &packet) noexcept {
        return static_cast<Impl *>(this)->writePacketImpl(packet);
    }

    /// @brief Write mixed packet: fixed-size header and dynamic-sized buffer
    /// @param header Header to write
    /// @param buffer Source buffer
    /// @return Result indicating success or error
    [[nodiscard]] ResultType writeMixed(trivial auto const &header, BytesView buffer) noexcept {
        return static_cast<Impl *>(this)->writeMixedImpl(header, buffer);
    }
};

}// namespace kf::mixin

#define KF_IMPL_BINARY_WRITABLE(...) friend struct kf::mixin::BinaryWritable<__VA_ARGS__>
