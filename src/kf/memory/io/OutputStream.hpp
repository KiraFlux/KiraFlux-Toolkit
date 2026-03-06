// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include <Stream.h>

#include "kf/Option.hpp"
#include "kf/aliases.hpp"

namespace kf::memory::io {

/// @brief Output stream for writing data to Stream interface
/// @note Wraps Arduino Stream class for type-safe writing with error handling
struct OutputStream {

private:
    Stream &_stream;///< Reference to underlying Stream object

public:
    /// @brief Construct output stream from Stream reference
    /// @param s Stream object to write to
    explicit OutputStream(Stream &s) noexcept :
        _stream{s} {}

    /// @brief Write single byte to stream
    /// @param byte Byte value to write
    /// @return true if byte successfully written
    [[nodiscard]] bool writeByte(u8 byte) noexcept {
        return _stream.write(byte) == 1;
    }

    /// @brief Write buffer of data to stream
    /// @param data Pointer to data buffer
    /// @param length Number of bytes to write
    /// @return true if all bytes successfully written
    [[nodiscard]] bool write(const void *data, usize length) noexcept {
        return _stream.write(static_cast<const u8 *>(data), length) == length;
    }

    /// @brief Write object of type T to stream
    /// @tparam T Type to write (must be trivially copyable)
    /// @param value Object to write
    /// @return true if object successfully written
    template<typename T> [[nodiscard]] inline bool write(const T &value) noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return write(static_cast<const void *>(&value), sizeof(T));
    }
};

}// namespace kf::memory::io