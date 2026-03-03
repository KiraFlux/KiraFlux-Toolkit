// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::ui::internal {

enum class ValuePlacement : char {
    Hidden, // Do not display value at all
    Outside,// Show value outside of widget bounds
    Inside, // Show value inside widget
};

}// namespace kf::ui::internal
