// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"

// autoconfig

#ifndef KF_CONFIG_MAIN_LOGGER_KEY
#define KF_CONFIG_MAIN_LOGGER_KEY "main"
#endif

#ifndef KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH
#define KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH 256
#endif

#ifndef KF_CONFIG_MAIN_UART_BAUDRATE
#define KF_CONFIG_MAIN_UART_BAUDRATE 115200
#endif

#ifndef KF_CONFIG_MAIN_UART_NUM
#define KF_CONFIG_MAIN_UART_NUM 0
#endif

//

namespace kf {

/// @brief App Context
struct Init;

/// @brief KiraFlux Toolkit application entry point
void main(Init &init);

namespace internal {

struct InitBase {

    /// @brief Application Logger
    Logger logger{
        (KF_CONFIG_MAIN_LOGGER_KEY),
        {_main_logger_buffer},
    };

private:
    char _main_logger_buffer[(KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH)]{};
};

}// namespace internal

}// namespace kf

#ifdef ARDUINO

#include <Arduino.h>

#include "kf/UART.hpp"

namespace kf {

struct Init : internal::InitBase {

    /// @brief Platform-depended standard binary Input/Output Stream
    UART io{
        _uart_config,
        (KF_CONFIG_MAIN_UART_NUM),
    };

private:
    UART::Config _uart_config{
        .baudrate = (KF_CONFIG_MAIN_UART_BAUDRATE),
    };
};

}// namespace kf

#define KF_PLATFORM_APP_FUNCTION_DECLARATION void setup()
#define KF_PLATFORM_APP_QUIT()

void loop() {}// not used

#else

#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/concepts.hpp"

#include "kf/mixin/BinaryReadable.hpp"
#include "kf/mixin/BinaryWritable.hpp"
#include "kf/mixin/Flush.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/ReadAvailable.hpp"
#include "kf/mixin/WriteAvailable.hpp"

#define KF_PLATFORM_APP_FUNCTION_DECLARATION int main()
#define KF_PLATFORM_APP_QUIT() (0)

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
    mixin::ReadAvailable<AppIO>,
    mixin::BinaryWritable<AppIO, AppIoWriteResult>,
    mixin::WriteAvailable<AppIO>,

    mixin::Flush<AppIO>

{
    using Self = AppIO;

    using Error = AppIoError;

    constexpr AppIO() noexcept = default;

private:
    termios kf_original_tty;

    KF_IMPL_INITABLE(Self, void());
    void initImpl() noexcept {
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
    }

    KF_IMPL_QUITABLE(Self);
    void quitImpl() noexcept {
        if (::isatty(STDIN_FILENO)) {
            ::fcntl(STDIN_FILENO, F_SETFL, ::fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);
            ::tcsetattr(STDIN_FILENO, TCSANOW, &kf_original_tty);
        }
    }

    KF_IMPL_BINARY_READABLE(Self, Error);

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<u8 const>, Error> {
        if (buffer.empty()) {
            return ok(Slice<u8 const>{});
        }

        auto const to_read = buffer.length();

        std::cin.read(reinterpret_cast<char *>(buffer.data()), to_read);
        auto const bytes_read = static_cast<usize>(std::cin.gcount());

        if (bytes_read == 0) {
            return error(Error::ReadFailed);
        }

        return ok(Slice<u8 const>{buffer.data(), bytes_read});
    }

    template<trivial T> auto readPacketImpl() -> Result<T, Error> {
        constexpr auto to_read = sizeof(T);

        if constexpr (to_read == sizeof(u8)) {
            int const c = std::cin.get();

            if (EOF == c) {
                return error(Error::ReadFailed);
            } else {
                return ok(static_cast<T>(c));
            }
        } else {
            T packet;

            std::cin.read(reinterpret_cast<char *>(&packet), to_read);
            auto const bytes_read = static_cast<usize>(std::cin.gcount());

            if (to_read == bytes_read) {
                return ok(packet);
            } else {
                return error(Error::ReadFailed);
            }
        }
    }

    KF_IMPL_READ_AVAILABLE(Self);
    usize availableForReadImpl() const noexcept {
        return std::cin.rdbuf()->in_avail();
    }

    KF_IMPL_BINARY_WRITABLE(Self, AppIoWriteResult);

    AppIoWriteResult writeBufferImpl(Slice<u8 const> buffer) noexcept {
        if (not buffer.empty()) {
            auto const to_write = buffer.length();

            if (not std::cout.write(reinterpret_cast<char const *>(buffer.data()), to_write).good()) {
                return error(Error::WriteFailed);
            }
        }

        return ok();
    }

    AppIoWriteResult writePacketImpl(trivial auto const &packet) noexcept {
        constexpr auto to_write = sizeof(packet);

        if constexpr (to_write == sizeof(u8)) {
            std::cout.put(static_cast<char>(packet));
        } else {
            std::cout.write(reinterpret_cast<char const *>(&packet), to_write);
        }

        if (not std::cout.good()) {
            return error(Error::WriteFailed);
        } else {
            return ok();
        }
    }

    AppIoWriteResult writeMixedImpl(trivial auto const &header, Slice<u8 const> buffer) noexcept {
        KF_TRY(this->writePacket(header));
        return this->writeBuffer(buffer);
    }

    KF_IMPL_WRITE_AVAILABLE(Self);
    usize availableForWriteImpl() const noexcept {
        return std::cout.good() ? static_cast<usize>(-1) : 0;
    }

    KF_IMPL_FLUSH(Self);
    void flushImpl() noexcept {
        (void) std::cout.flush();
    }
};

}// namespace internal

struct Init : internal::InitBase {

    /// @brief Platform-depended standard binary Input/Output Stream
    internal::AppIO io{};
};

}// namespace kf

#endif

KF_PLATFORM_APP_FUNCTION_DECLARATION {

    static kf::Init init{};

    init.io.init();

    kf::Logger::writer = [](auto str) {
        (void) init.io.writeBuffer({reinterpret_cast<kf::u8 const *>(str.data()), str.length()});
        init.io.flush();
    };

    init.logger.info("KiraFlux Toolkit v0.3.5");

    main(init);

    init.io.quit();

    return KF_PLATFORM_APP_QUIT();
}

// cleanup

#undef KF_PLATFORM_APP_FUNCTION_DECLARATION
#undef KF_PLATFORM_APP_QUIT

#undef KF_CONFIG_MAIN_LOGGER_KEY
#undef KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH
#undef KF_CONFIG_MAIN_UART_BAUDRATE
