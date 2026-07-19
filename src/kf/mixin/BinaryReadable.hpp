// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/concepts.hpp"
#include "kf/primitives.hpp"

namespace kf::mixin {

struct BinaryReadableTag {};

/// @brief CRTP base class for readable byte streams
/// @tparam Impl      Derived class
/// @tparam ErrorImpl Error type used by the implementation
/// @note Derived classes must implement:
///
///       - `Result<Slice<const u8>, Error> readBufferImpl(Slice<u8> buffer) noexcept`
///         Read up to buffer.size() bytes into the given buffer. Returns a slice of actually read data
///
///       - `template<typename T> Result<T, Error> readPacketImpl() noexcept`
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
    [[nodiscard]] auto readBuffer(Slice<u8> buffer) noexcept -> Result<Slice<const u8>, ErrorImpl> {
        return impl().readBufferImpl(buffer);
    }

    /// @brief Read fixed‑size buffer
    /// @tparam T Type of packet
    /// @return Packet value or error
    template<trivial T> [[nodiscard]] auto readPacket() noexcept -> Result<T, ErrorImpl> {
        return impl().template readPacketImpl<T>();
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::mixin

#define KF_IMPL_BINARY_READABLE(...) friend struct kf::mixin::BinaryReadable<__VA_ARGS__>
