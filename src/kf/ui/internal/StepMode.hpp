// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf {// NOLINT(*-concat-nested-namespaces) // for c++11 capability
namespace ui {// NOLINT(*-concat-nested-namespaces)
namespace internal {// NOLINT(*-concat-nested-namespaces)

enum class StepMode : char {
    Arithmetic,            ///< Add/subtract step value
    ArithmeticPositiveOnly,///< Add/subtract step, clamp at zero
    Geometric              ///< Multiply/divide by step value
};

}
}
}// namespace kf