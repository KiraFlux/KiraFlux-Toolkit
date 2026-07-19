// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <utility>

#include "kf/Slice.hpp"
#include "kf/primitives.hpp"

namespace kf::mixin {

struct BinaryWritableTag {};

/// @brief CRTP base class for writable streams
/// @tparam Impl      Derived class
/// @tparam ErrorImpl Error type used by the implementation
/// @note Derived classes must implement:
///
///       - `ResultType writeBufferImpl(Slice<const u8> buffer) noexcept`
///         Write a contiguous buffer of bytes
///
///       - `ResultType writePacketImpl(auto &&packet) noexcept`
///         Write a trivially copyable object
///
///       - `ResultType writeMixedImpl(auto &&header, Slice<const u8> buffer) noexcept`
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
    [[nodiscard]] ResultType writeBuffer(Slice<const u8> buffer) noexcept {
        return impl().writeBufferImpl(buffer);
    }

    /// @brief Write fixed‑size packet
    /// @param packet Value to write
    /// @return Result indicating success or error
    [[nodiscard]] ResultType writePacket(auto &&packet) noexcept {
        static_assert(std::is_trivially_copyable_v<std::decay_t<decltype(packet)>>, "packet must be trivially copyable");// TODO: use concept constraint
        return impl().writePacketImpl(std::forward<decltype(packet)>(packet));
    }

    /// @brief Write mixed packet: fixed-size header and dynamic-sized buffer
    /// @param header Header to write
    /// @param buffer Source buffer
    /// @return Result indicating success or error
    [[nodiscard]] ResultType writeMixed(auto &&header, Slice<const u8> buffer) noexcept {
        static_assert(std::is_trivially_copyable_v<std::decay_t<decltype(header)>>, "header must be trivially copyable");
        return impl().writeMixedImpl(std::forward<decltype(header)>(header), buffer);
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::mixin

#define KF_IMPL_BINARY_WRITABLE(...) friend struct kf::mixin::BinaryWritable<__VA_ARGS__>
