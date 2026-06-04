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
        tracked_constructor_destructor{true},
        tracked_copy{true},
        tracked_move{false};

    static constexpr bool
        copyable{true},
        movable{false};

    int value;

    explicit OnlyCopyable(int value) noexcept : value{value} {
        constructor_calls += 1;
    }

    OnlyCopyable(const OnlyCopyable &other) noexcept : value{other.value} {
        copy_constructor_calls += 1;
    }

    OnlyCopyable &operator=(const OnlyCopyable &other) noexcept {
        copy_assignment_calls += 1;

        return *this;
    }

    OnlyCopyable(OnlyCopyable &&) noexcept = default;

    OnlyCopyable &operator=(OnlyCopyable &&) noexcept = default;

    ~OnlyCopyable() noexcept {
        destructor_calls += 1;
    }

    bool operator==(const OnlyCopyable &other) const noexcept {
        return value == other.value;
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
        tracked_constructor_destructor{true},
        tracked_copy{false},
        tracked_move{true};

    static constexpr bool
        copyable{false},
        movable{true};

    int value;

    explicit OnlyMovable(int value) noexcept : value(value) {
        constructor_calls += 1;
    }

    OnlyMovable(OnlyMovable &&other) noexcept : value(other.value) {
        move_constructor_calls += 1;

        other.value = 0;
    }

    OnlyMovable &operator=(OnlyMovable &&other) noexcept {
        move_assignment_calls += 1;

        value = other.value;
        other.value = 0;

        return *this;
    }

    OnlyMovable(const OnlyMovable &) noexcept = delete;

    OnlyMovable &operator=(const OnlyMovable &) noexcept = delete;

    ~OnlyMovable() noexcept {
        destructor_calls += 1;
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
        tracked_constructor_destructor{true},
        tracked_copy{true},
        tracked_move{true};

    static constexpr bool
        copyable{true},
        movable{true};

    int value;

    explicit CopyableMovable(int value) noexcept : value{value} {
        constructor_calls += 1;
    }

    CopyableMovable(const CopyableMovable &other) noexcept : value{other.value} {
        copy_constructor_calls += 1;
    }

    CopyableMovable(CopyableMovable &&other) noexcept : value(other.value) {
        move_constructor_calls += 1;

        other.value = 0;
    }

    CopyableMovable &operator=(const CopyableMovable &other) noexcept {
        copy_assignment_calls += 1;

        return *this;
    }

    CopyableMovable &operator=(CopyableMovable &&other) noexcept {
        move_assignment_calls += 1;

        value = other.value;
        other.value = 0;

        return *this;
    }

    ~CopyableMovable() noexcept {
        destructor_calls += 1;
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
        tracked_constructor_destructor{false},
        tracked_copy{false},
        tracked_move{false};

    static constexpr bool
        copyable{true},
        movable{true};

    static void reset() noexcept {}

    int value;
};

}// namespace kf::test