// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/primitives.hpp"

namespace kf::mixin {

struct ReadableTag {};

/// @brief CRTP base class for readable streams (e.g. I2C, SPI, UART).
/// @tparam Impl      Derived class.
/// @tparam ErrorImpl Error type used by the implementation.
/// @note Derived classes must implement:
///
///       - `Result<Slice<const u8>, Error> readBufferImpl(Slice<u8> buffer) noexcept`
///         Read up to buffer.size() bytes into the given buffer. Returns a slice of actually read data.
///
///       - `template<typename T> Result<T, Error> readPacketImpl() noexcept`
///         Read a trivially copyable object of type T (size fixed at compile time).
template<typename Impl, typename ErrorImpl> struct Readable : ReadableTag {

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
    /// @tparam T Type of packet (trivially copyable)
    /// @return Packet value or error
    template<typename T> [[nodiscard]] auto readPacket() noexcept -> Result<T, ErrorImpl> {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return impl().template readPacketImpl<T>();
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::mixin

#define KF_IMPL_READABLE(...) friend struct kf::mixin::Readable<__VA_ARGS__>
