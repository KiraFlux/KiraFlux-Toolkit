// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/Stack.hpp"
#include "kf/StringView.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Writable.hpp"

namespace kf {

/// @brief Logging system for embedded applications
struct Logger :

    mixin::NonCopyable,
    private mixin::Writable<Logger, char>

{

    using WriteHandler = void (*)(StringView);

    /// @brief Current output handler (nullptr disables logging)
    inline static WriteHandler writer{nullptr};

    explicit constexpr Logger(StringView key, Slice<char> buffer = {}) noexcept :
        _key{key}, _stack{buffer} {}

    /// @brief Get Logger key
    [[nodiscard]] constexpr StringView key() const noexcept {
        return _key;
    }

#define MAKE(__kf_level__, __kf_level_name__)                                                                                \
    template<typename... Args> void __kf_level__(const internal::FormatString<Args...> &fmt, const Args &...args) noexcept { \
        this->append('[');                                                                                                   \
        this->append(_key);                                                                                                  \
        this->append(":" __kf_level_name__ "] ");                                                                            \
        this->format(fmt, args...);                                                                                          \
        this->append('\n');                                                                                                  \
        flush();                                                                                                             \
    }

    MAKE(debug, "D")
    MAKE(info, "I")
    MAKE(warn, "W")
    MAKE(error, "E")

#undef MAKE

    /// @brief Write all buffered chars
    void flush() noexcept {
        writer({_stack.slice()});
        _stack.reset();
    }

private:
    Stack<char> _stack;
    StringView _key;

    KF_IMPL_WRITABLE(Logger, char);
    bool writeImpl(char c) noexcept {
        if (nullptr == writer) {
            return false;
        }

        // bufferless mode -> direct write
        if (_stack.capacity() == 0) {
            writer({&c, 1});
        } else {
            if (_stack.full()) {
                flush();
            }

            (void) _stack.write(c);
        }

        return true;
    }
};

}// namespace kf