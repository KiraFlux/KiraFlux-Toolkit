#pragma once

#include <Stream.h>

#include "kf/Option.hpp"
#include "kf/aliases.hpp"
#include "kf/core/attributes.hpp"


namespace kf::mizlang {

/// @brief Input stream for reading data from Stream interface
/// @note Wraps Arduino Stream class for type-safe reading with error handling
struct InputStream {

private:
    Stream &stream;///< Reference to underlying Stream object

public:
    /// @brief Construct input stream from Stream reference
    /// @param s Stream object to read from
    explicit InputStream(Stream &s) noexcept:
        stream{s} {}

    /// @brief Discard all available data in stream
    void clean() noexcept {
        while (stream.available()) {
            (void) stream.read();
        }
    }

    /// @brief Get number of bytes available for reading
    /// @return Count of bytes available in stream
    kf_nodiscard usize available() noexcept {
        return stream.available();
    }

    /// @brief Read single byte from stream
    /// @return Optional byte value (empty if no data available)
    kf_nodiscard Option<u8> readByte() noexcept {
        const auto result = stream.read();

        if (result == -1) {
            return {};
        } else {
            return {static_cast<u8>(result)};
        }
    }

    /// @brief Read object of type T from stream
    /// @tparam T Type to read (must be trivially copyable)
    /// @return Optional object (empty if insufficient data available)
    template<typename T> kf_nodiscard Option<T> read() noexcept {
        T value;

        const usize bytes_read = stream.readBytes(
            reinterpret_cast<u8 *>(&value),
            sizeof(T));

        if (bytes_read == sizeof(T)) {
            return {value};
        } else {
            return {};
        }
    }
};

/// @brief Output stream for writing data to Stream interface
/// @note Wraps Arduino Stream class for type-safe writing with error handling
struct OutputStream {

private:
    Stream &stream;///< Reference to underlying Stream object

public:
    /// @brief Construct output stream from Stream reference
    /// @param s Stream object to write to
    explicit OutputStream(Stream &s) noexcept:
        stream{s} {}

    /// @brief Write single byte to stream
    /// @param byte Byte value to write
    /// @return true if byte successfully written
    kf_nodiscard bool writeByte(u8 byte) noexcept {
        return stream.write(byte) == 1;
    }

    /// @brief Write buffer of data to stream
    /// @param data Pointer to data buffer
    /// @param length Number of bytes to write
    /// @return true if all bytes successfully written
    kf_nodiscard bool write(const void *data, usize length) noexcept {
        return stream.write(static_cast<const u8 *>(data), length) == length;
    }

    /// @brief Write object of type T to stream
    /// @tparam T Type to write (must be trivially copyable)
    /// @param value Object to write
    /// @return true if object successfully written
    template<typename T> kf_nodiscard inline bool write(const T &value) noexcept {
        return write(static_cast<const void *>(&value), sizeof(T));
    }
};

}// namespace kf::mizlang