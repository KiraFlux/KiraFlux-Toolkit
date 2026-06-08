// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::internal {

enum class Placement : char {
    Hidden, ///< Do not display at all
    Outside,///< Show outside of bounds
    Inside, ///< Show inside
};

}// namespace kf::internal