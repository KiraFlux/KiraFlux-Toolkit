// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    UART.hpp

#pragma once

#ifdef ARDUINO
#include <HardwareSerial.h>
#endif

#include "kf/Bytes.hpp"
#include "kf/BytesView.hpp"
#include "kf/Result.hpp"
#include "kf/core.hpp"

#include "kf/mixin/BinaryReadable.hpp"
#include "kf/mixin/BinaryWritable.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Flush.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/ReadAvailable.hpp"
#include "kf/mixin/WriteAvailable.hpp"

namespace kf {

namespace internal {

enum class UartError {
    ReadFailed,
    WriteFailed,
};

struct UartConfig {
    u32 baudrate;
};

using UartWriteResult = Result<void, UartError>;

template<typename Impl> struct UartBase :

    mixin::NonCopyable,
    mixin::Initable<Impl, void()>,
    mixin::Quitable<Impl>,

    mixin::BinaryReadable<Impl, UartError>,
    mixin::ReadAvailable<Impl>,

    mixin::BinaryWritable<Impl, UartWriteResult>,
    mixin::WriteAvailable<Impl>,

    mixin::Flush<Impl>,

    mixin::Configured<UartConfig>

{
    using mixin::Configured<UartConfig>::Configured;
};

}// namespace internal

/// @brief Universal Asynchronous Receiver-Transmitter (UART) abstraction.
/// @note Provides buffered I/O with availability checks and flush support.
///       On Arduino, uses HardwareSerial. On native, acts as a no-op stub.
struct UART;

#ifdef ARDUINO

struct UART : internal::UartBase<UART> {
    using Self = UART;

    using Error = internal::UartError;
    using Config = internal::UartConfig;

    explicit UART(Config const &config, u8 uart_num) noexcept :
        internal::UartBase<UART>{config}, _serial{uart_num} {}

private:
    mutable HardwareSerial _serial;

    KF_IMPL_INITABLE(Self, void());
    void initImpl() noexcept {
        _serial.begin(this->config().baudrate);
    }

    KF_IMPL_QUITABLE(Self);
    void quitImpl() noexcept {
        _serial.end();
    }

    KF_IMPL_BINARY_READABLE(Self, Error);

    auto readBufferImpl(Bytes buffer) noexcept -> Result<BytesView, Error> {
        if (buffer.empty()) {
            return ok(BytesView{});
        }

        auto const bytes_read = _serial.readBytes(buffer.data(), buffer.length());

        if (bytes_read == 0) {
            return error(Error::ReadFailed);
        }

        return ok(BytesView{buffer.data(), bytes_read});
    }

    template<typename T> auto readPacketImpl() noexcept -> Result<T, Error> {
        constexpr auto to_read = sizeof(T);

        if constexpr (to_read == sizeof(u8)) {
            int const c = _serial.read();

            if ((-1) == c) {
                return error(Error::ReadFailed);
            }

            return ok(static_cast<T>(c));

        } else {
            T packet;

            auto const bytes_read = _serial.readBytes(reinterpret_cast<u8 *>(&packet), to_read);

            if (to_read != bytes_read) {
                return error(Error::ReadFailed);
            }

            return ok(packet);
        }
    }

    KF_IMPL_READ_AVAILABLE(Self);
    usize availableForReadImpl() const noexcept {
        return _serial.available();
    }

    KF_IMPL_BINARY_WRITABLE(Self, internal::UartWriteResult);

    auto writeBufferImpl(BytesView buffer) noexcept -> internal::UartWriteResult {
        if (not buffer.empty()) {
            auto const to_write = buffer.length();

            if (_serial.write(buffer.data(), to_write) != to_write) {
                return error(Error::WriteFailed);
            }
        }

        return ok();
    }

    auto writePacketImpl(trivial auto const &packet) noexcept -> internal::UartWriteResult {
        constexpr auto to_write = sizeof(packet);

        usize written;

        if constexpr (to_write == sizeof(u8)) {
            written = _serial.write(static_cast<u8>(packet));
        } else {
            written = _serial.write(reinterpret_cast<u8 const *>(&packet), to_write);
        }

        if (to_write != written) {
            return error(Error::WriteFailed);
        } else {
            return ok();
        }
    }

    auto writeMixedImpl(trivial auto const &header, BytesView buffer) noexcept -> internal::UartWriteResult {
        KF_TRY(this->writePacket(header));
        return this->writeBuffer(buffer);
    }

    KF_IMPL_WRITE_AVAILABLE(Self);
    usize availableForWriteImpl() const noexcept {
        auto const ret = _serial.availableForWrite();
        // Arduino Stream returns size 0 if "may write any size"
        return (0 == ret) ? static_cast<usize>(-1) : ret;
    }

    KF_IMPL_FLUSH(Self);
    void flushImpl() noexcept {
        _serial.flush();
    }
};

#else

struct UART : internal::UartBase<UART> {
    using Self = UART;

    using Error = internal::UartError;
    using Config = internal::UartConfig;

    explicit UART(Config const &config, u8 uart_num) noexcept :
        internal::UartBase<UART>{config} {
        (void) uart_num;
    }

private:
    KF_IMPL_INITABLE(Self, void());
    void initImpl() noexcept {}

    KF_IMPL_QUITABLE(Self);
    void quitImpl() noexcept {}

    KF_IMPL_BINARY_READABLE(Self, Error);

    auto readBufferImpl(Bytes buffer) noexcept -> Result<BytesView, Error> {
        (void) buffer;

        return error(Error::ReadFailed);
    }

    template<typename T> auto readPacketImpl() noexcept -> Result<T, Error> {
        return error(Error::ReadFailed);
    }

    KF_IMPL_READ_AVAILABLE(Self);
    usize availableForReadImpl() noexcept {
        return 0;
    }

    KF_IMPL_BINARY_WRITABLE(Self, internal::UartWriteResult);

    auto writeBufferImpl(BytesView buffer) noexcept -> internal::UartWriteResult {
        (void) buffer;

        return ok();
    }

    auto writePacketImpl(trivial auto const &packet) noexcept -> internal::UartWriteResult {
        (void) packet;

        return ok();
    }

    auto writeMixedImpl(trivial auto const &header, BytesView buffer) noexcept -> internal::UartWriteResult {
        (void) header;
        (void) buffer;

        return ok();
    }

    KF_IMPL_WRITE_AVAILABLE(Self);
    usize availableForWriteImpl() noexcept {
        return static_cast<usize>(-1);
    }

    KF_IMPL_FLUSH(Self);
    void flushImpl() noexcept {}
};

#endif

}// namespace kf