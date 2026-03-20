// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/Slice.hpp"

namespace kf::memory::io {

struct ReadableTag {};

/// @brief CRTP base class for readable streams (e.g. I2C, SPI, UART).
/// @tparam Impl      Derived class.
/// @tparam ErrorImpl Error type used by the implementation.
/// @note Derived classes must implement:
///
///       - `Result<memory::Slice<const u8>, Error> readBufferImpl(memory::Slice<u8>)`
///         Read up to buffer.size() bytes into the given buffer. Returns a slice of actually read data.
///
///       - `template<typename T> Result<T, Error> readPacketImpl()`
///         Read a trivially copyable object of type T (size fixed at compile time).
template<typename Impl, typename ErrorImpl> struct Readable : ReadableTag {

    /// @brief Read single byte
    /// @return Byte value or error
    [[nodiscard]] Result<u8, ErrorImpl> readByte() noexcept {
        return this->readPacket<u8>();
    }

    /// @brief Read arbitrary number of bytes into buffer
    /// @param buffer Destination buffer
    /// @return Slice containing actually read data, or error
    [[nodiscard]] Result<memory::Slice<const u8>, ErrorImpl> readBuffer(memory::Slice<u8> buffer) noexcept {
        return impl().readBufferImpl(buffer);
    }

    /// @brief Read fixed‑size buffer
    /// @tparam T Type of packet (trivially copyable)
    /// @return Packet value or error
    template<typename T> [[nodiscard]] Result<T, ErrorImpl> readPacket() noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return impl().template readPacketImpl<T>();
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::memory::io

#define KF_IMPL_READABLE(__impl__, __error_type__) friend struct kf::memory::io::Readable<__impl__, __error_type__>