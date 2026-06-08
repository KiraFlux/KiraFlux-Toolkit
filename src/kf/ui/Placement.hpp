// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::ui {

enum class Placement : char {
    Hidden, ///< Do not display at all
    Outside,///< Show outside of bounds
    Inside, ///< Show inside
};

}// namespace kf::ui