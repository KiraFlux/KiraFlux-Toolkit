// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

namespace kf::test {

struct OnlyCopyable {
    inline static int
        constructor_calls{0},
        copy_constructor_calls{0},
        copy_assignment_calls{0},
        destructor_calls{0};

    static constexpr bool
        tracked_contstructor_destructor{true},
        tracked_copy{true},
        tracked_move{false};

    int _value;

    explicit OnlyCopyable(int value) noexcept : _value{value} {
        constructor_calls += 1;
    }

    OnlyCopyable(const OnlyCopyable &other) noexcept : _value{other._value} {
        copy_constructor_calls += 1;
    }

    OnlyCopyable &operator=(const OnlyCopyable &other) noexcept {
        copy_assignment_calls += 1;

        return *this;
    }

    OnlyCopyable(OnlyCopyable &&) noexcept = delete;

    OnlyCopyable &operator=(OnlyCopyable &&) noexcept = delete;

    ~OnlyCopyable() noexcept {
        destructor_calls += 1;
    }

    bool operator==(const OnlyCopyable &other) const noexcept {
        return _value == other._value;
    }

    static void reset() noexcept {
        constructor_calls = 0;

        copy_constructor_calls = 0;
        copy_assignment_calls = 0;

        destructor_calls = 0;
    }
};

struct OnlyMovable {
    inline static int
        constructor_calls{0},
        move_constructor_calls{0},
        move_assignment_calls{0},
        destructor_calls{0};

    static constexpr bool
        tracked_contstructor_destructor{true},
        tracked_copy{false},
        tracked_move{true};

    int _value;

    explicit OnlyMovable(int value) noexcept : _value(value) {
        constructor_calls += 1;
    }

    OnlyMovable(OnlyMovable &&other) noexcept : _value(other._value) {
        move_constructor_calls += 1;

        other._value = 0;
    }

    OnlyMovable &operator=(OnlyMovable &&other) noexcept {
        move_assignment_calls += 1;

        _value = other._value;
        other._value = 0;

        return *this;
    }

    OnlyMovable(const OnlyMovable &) noexcept = delete;

    OnlyMovable &operator=(const OnlyMovable &) noexcept = delete;

    ~OnlyMovable() noexcept {
        destructor_calls += 1;
    }

    bool operator==(const OnlyMovable &other) const noexcept {
        return _value == other._value;
    }

    static void reset() {
        constructor_calls = 0;

        move_constructor_calls = 0;
        move_assignment_calls = 0;

        destructor_calls = 0;
    }
};

struct CopyableMovable {
    inline static int
        constructor_calls{0},
        copy_constructor_calls{0},
        copy_assignment_calls{0},
        move_constructor_calls{0},
        move_assignment_calls{0},
        destructor_calls{0};

    static constexpr bool
        tracked_contstructor_destructor{true},
        tracked_copy{true},
        tracked_move{true};

    int _value;

    explicit CopyableMovable(int value) noexcept : _value{value} {
        constructor_calls += 1;
    }

    CopyableMovable(const CopyableMovable &other) noexcept : _value{other._value} {
        copy_constructor_calls += 1;
    }

    CopyableMovable(CopyableMovable &&other) noexcept : _value(other._value) {
        move_constructor_calls += 1;

        other._value = 0;
    }

    CopyableMovable &operator=(const CopyableMovable &other) noexcept {
        copy_assignment_calls += 1;

        return *this;
    }

    CopyableMovable &operator=(CopyableMovable &&other) noexcept {
        move_assignment_calls += 1;

        _value = other._value;
        other._value = 0;

        return *this;
    }

    ~CopyableMovable() noexcept {
        destructor_calls += 1;
    }

    bool operator==(const CopyableMovable &other) const noexcept {
        return _value == other._value;
    }

    static void reset() noexcept {
        constructor_calls = 0;

        copy_constructor_calls = 0;
        copy_assignment_calls = 0;

        move_constructor_calls = 0;
        move_assignment_calls = 0;

        destructor_calls = 0;
    }
};

struct TrivialType {
    static constexpr bool
        tracked_contstructor_destructor{false},
        tracked_copy{false},
        tracked_move{false};

    int value;

    bool operator==(const TrivialType &other) const noexcept {
        return value == other.value;
    }
};

}// namespace kf::test