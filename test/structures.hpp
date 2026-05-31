// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::test {

struct Point {
    int x, y;
    static Point create() noexcept { return Point{10, 20}; }
    bool operator==(const Point &other) const { return x == other.x and y == other.y; }
};

}// namespace kf::test