// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"
#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/StringView.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/BinaryReadable.hpp"
#include "kf/mixin/BinaryWritable.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"

// autoconfig

#ifndef KF_CONFIG_MAIN_LOGGER_KEY
#define KF_CONFIG_MAIN_LOGGER_KEY "main"
#endif

#ifndef KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH
#define KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH 256
#endif

#ifndef KF_CONFIG_SERIAL_BAUDRATE
#define KF_CONFIG_SERIAL_BAUDRATE 115200
#endif

//

#ifdef ARDUINO

#include <Arduino.h>

#define KF_PLATFORM_APP_IO_READ_CHAR_FAILED (-1)
#define KF_PLATFORM_APP_FUNCTION_DECLARATION void setup()
#define KF_PLATFORM_APP_QUIT()

void loop() {}// not used

#else

#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

#define KF_PLATFORM_APP_IO_READ_CHAR_FAILED EOF
#define KF_PLATFORM_APP_FUNCTION_DECLARATION int main()

inline termios kf_original_tty;

#define KF_PLATFORM_APP_IO_READ_CHAR_FAILED EOF
#define KF_PLATFORM_APP_FUNCTION_DECLARATION int main()

#define KF_PLATFORM_APP_QUIT() (0)

#endif

namespace kf {

namespace internal {

enum class AppIoError : char {
    ReadFailed,
    WriteFailed,
};

using AppIoWriteResult = Result<void, AppIoError>;

struct AppIO :

    mixin::NonCopyable,
    mixin::Initable<AppIO, void()>,
    mixin::Quitable<AppIO>,
    mixin::BinaryReadable<AppIO, AppIoError>,
    mixin::BinaryWritable<AppIO, AppIoWriteResult>

{
    using Self = AppIO;

    using Error = AppIoError;

    constexpr AppIO() noexcept = default;

    [[nodiscard]] usize availableForRead() noexcept {
        return (
#ifdef ARDUINO
            _serial.available()
#else
            std::cin.rdbuf()->in_avail()
#endif
        );
    }

    [[nodiscard]] usize availableForWrite() noexcept {
#ifdef ARDUINO
        auto const ret = _serial.availableForWrite();
        // Arduino Stream returns size 0 if "may write any size"
        return (0 == ret) ? static_cast<usize>(-1) : ret;
#else
        return std::cout.good() ? static_cast<usize>(-1) : 0;
#endif
    }

    void flush() noexcept {
#ifdef ARDUINO
        _serial.flush();
#else
        (void) std::cout.flush();
#endif
    }

private:
#ifdef ARDUINO
    HardwareSerial _serial{0};
#endif

    KF_IMPL_INITABLE(Self, void());
    void initImpl() noexcept {
#ifdef ARDUINO
        _serial.begin((KF_CONFIG_SERIAL_BAUDRATE));
#else
        if (::isatty(STDIN_FILENO)) {

            /* 1. enable raw mode (like arduino UART) */
            ::tcgetattr(STDIN_FILENO, &kf_original_tty);
            termios raw = kf_original_tty;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_cc[VMIN] = 0;  /* 0 - nonblocking is no bytes */
            raw.c_cc[VTIME] = 0; /* no timeouts */
            ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

            /* 2. change stdin handler to async mode */
            int flags = ::fcntl(STDIN_FILENO, F_GETFL, 0);
            ::fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        }

        /* Disable sync iostream with C functions for correct std::cin.get() work in raw mode */
        std::ios_base::sync_with_stdio(false);
#endif
    }

    KF_IMPL_QUITABLE(Self);
    void quitImpl() noexcept {
#ifdef ARDUINO
        _serial.end();
#else
        if (::isatty(STDIN_FILENO)) {
            ::fcntl(STDIN_FILENO, F_SETFL, ::fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);
            ::tcsetattr(STDIN_FILENO, TCSANOW, &kf_original_tty);
        }
#endif
    }

    KF_IMPL_BINARY_READABLE(Self, Error);

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<u8 const>, Error> {
        if (buffer.empty()) {
            return ok(Slice<u8 const>{});
        }

        auto const to_read = buffer.length();
        usize bytes_read;

#ifdef ARDUINO
        bytes_read = _serial.readBytes(reinterpret_cast<char *>(buffer.data()), to_read);
#else
        std::cin.read(reinterpret_cast<char *>(buffer.data()), to_read);
        bytes_read = static_cast<usize>(std::cin.gcount());
#endif

        if (bytes_read == 0) {
            return error(Error::ReadFailed);
        }

        return ok(Slice<u8 const>{buffer.data(), bytes_read});
    }

    template<trivial T> auto readPacketImpl() -> Result<T, Error> {
        constexpr auto to_read = sizeof(T);

        if constexpr (to_read == sizeof(u8)) {
            int const c = (
#ifdef ARDUINO
                _serial.read()
#else
                std::cin.get()
#endif
            );

            if (KF_PLATFORM_APP_IO_READ_CHAR_FAILED == c) {
                return error(Error::ReadFailed);
            } else {
                return ok(static_cast<T>(c));
            }
        } else {
            T packet;

            usize bytes_read;
#ifdef ARDUINO
            bytes_read = _serial.readBytes(reinterpret_cast<u8 *>(&packet), to_read);
#else
            std::cin.read(reinterpret_cast<char *>(&packet), to_read);
            bytes_read = static_cast<usize>(std::cin.gcount());
#endif
            if (to_read == bytes_read) {
                return ok(packet);
            } else {
                return error(Error::ReadFailed);
            }
        }
    }

    KF_IMPL_BINARY_WRITABLE(Self, AppIoWriteResult);

    AppIoWriteResult writeBufferImpl(Slice<u8 const> buffer) noexcept {
        if (not buffer.empty()) {
            auto const to_write = buffer.length();

            if (
#ifdef ARDUINO
                _serial.write(buffer.data(), to_write) != to_write
#else
                not std::cout.write(reinterpret_cast<char const *>(buffer.data()), to_write).good()
#endif
            ) {
                return error(Error::WriteFailed);
            }
        }

        return ok();
    }

    AppIoWriteResult writePacketImpl(trivial auto const &packet) noexcept {
        constexpr auto to_write = sizeof(packet);

#ifdef ARDUINO
        usize written;

        if constexpr (to_write == sizeof(u8)) {
            written = _serial.write(static_cast<u8>(packet));
        } else {
            written = _serial.write(reinterpret_cast<u8 const *>(&packet), to_write);
        }

        if (to_write != written)
#else

        if constexpr (to_write == sizeof(u8)) {
            std::cout.put(static_cast<char>(packet));
        } else {
            std::cout.write(reinterpret_cast<char const *>(&packet), to_write);
        }

        if (not std::cout.good())
#endif
        {
            return error(Error::WriteFailed);
        } else {
            return ok();
        }
    }

    AppIoWriteResult writeMixedImpl(trivial auto const &header, Slice<u8 const> buffer) noexcept {
        KF_TRY(this->writePacket(header));
        return this->writeBuffer(buffer);
    }
};

}// namespace internal

/// @brief App Context
struct Init {

    /// @brief Platform-depended standard binary Input/Output Stream
    internal::AppIO io;

    /// @brief Application Logger
    Logger logger;
};

/// @brief KiraFlux Toolkit application entry point
void main(Init &init);

}// namespace kf

KF_PLATFORM_APP_FUNCTION_DECLARATION {
    static char main_logger_buffer[(KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH)]{};

    static kf::Init init{
        .io = {},
        .logger = kf::Logger{
            (KF_CONFIG_MAIN_LOGGER_KEY),
            {main_logger_buffer},
        },
    };

    init.io.init();

    kf::Logger::writer = [](kf::StringView str) {
        (void) init.io.writeBuffer({reinterpret_cast<kf::u8 const *>(str.data()), str.length()});
        init.io.flush();
    };

    init.logger.info("KiraFlux Toolkit v0.3.5");

    main(init);

    init.io.quit();

    return KF_PLATFORM_APP_QUIT();
}

// cleanup

#undef KF_PLATFORM_APP_IO_READ_CHAR_FAILED

#undef KF_PLATFORM_APP_FUNCTION_DECLARATION
#undef KF_PLATFORM_APP_QUIT

#undef KF_CONFIG_MAIN_LOGGER_KEY
#undef KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH
#undef KF_CONFIG_SERIAL_BAUDRATE
