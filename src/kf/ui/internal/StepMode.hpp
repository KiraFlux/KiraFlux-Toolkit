// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::ui::internal {

enum class StepMode : char {
    Arithmetic,            ///< Add/subtract step value
    ArithmeticPositiveOnly,///< Add/subtract step, clamp at zero
    Geometric              ///< Multiply/divide by step value
};

}// namespace kf::ui::internal