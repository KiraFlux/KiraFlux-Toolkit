// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <Stream.h>

#include "kf/primitives.hpp"
#include "kf/io/Readable.hpp"
#include "kf/io/Writable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::io {

namespace internal {

enum class ArduinoStreamError : u8 {
    ReadNotAvailable,
    ReadFalied,
    WriteFailed,
};

}

struct ArduinoStream final : mixin::NonCopyable,
                             io::Readable<ArduinoStream, internal::ArduinoStreamError>,
                             io::Writable<ArduinoStream, internal::ArduinoStreamError> {

    using Error = internal::ArduinoStreamError;

    constexpr explicit ArduinoStream(Stream &stream) noexcept : _stream{stream} {}

private:
    Stream &_stream;

    // impl
    using This = ArduinoStream;

    KF_IMPL_READABLE(This, Error);

    Result<memory::Slice<const u8>, Error> readBufferImpl(memory::Slice<u8> dest) noexcept {
        constexpr auto min_available{1u};

        if (_stream.available() < min_available) { return {Error::ReadNotAvailable}; }

        const auto readed = _stream.readBytes(dest.data(), dest.size());
        if (readed < min_available) { return {Error::ReadFalied}; }

        return {memory::Slice<const u8>{dest.data(), readed}};
    }

    template<typename T> Result<T, Error> readPacketImpl() noexcept {
        constexpr auto to_read{sizeof(T)};
        if (_stream.available() < to_read) { return {Error::ReadNotAvailable}; }

        if constexpr (to_read == sizeof(u8)) {
            const auto read_result = _stream.read();
            if (-1 == read_result) {
                return {Error::ReadFalied};
            } else {
                return {static_cast<T>(read_result)};
            }
        } else {
            T dest;
            const auto readed = _stream.readBytes(reinterpret_cast<u8 *>(&dest), to_read);
            if (to_read == readed) {
                return {dest};
            } else {
                return {Error::ReadFalied};
            }
        }
    }

    KF_IMPL_WRITABLE(This, Error);

    Result<void, Error> writeBufferImpl(memory::Slice<const u8> buffer) noexcept {
        const auto to_write = buffer.size();
        // _stream.availableForWrite() ?
        if (_stream.write(buffer.data(), to_write) != to_write) { return {Error::WriteFailed}; }

        return {};
    }

    template<typename T> Result<void, Error> writePacketImpl(T &&packet) noexcept {
        constexpr auto to_write = sizeof(packet);
        usize written;

        if constexpr (to_write == sizeof(u8)) {
            written = _stream.write(static_cast<u8>(packet));
        } else {
            written = _stream.write(reinterpret_cast<const u8 *>(&packet), to_write);
        }

        if (to_write != written) { return {Error::WriteFailed}; }

        return {};
    }

    template<typename T> Result<void, Error> writeMixedImpl(T &&header, memory::Slice<const u8> buffer) noexcept {
        const auto header_result = this->writePacket(std::forward<T>(header));
        if (header_result.isError()) { return header_result; }

        return this->writeBuffer(buffer);
    }
};

}// namespace kf::io