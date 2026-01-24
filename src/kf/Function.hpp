// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>


namespace kf {

/// Function wrapper for platforms with standard library support
template<typename F> using Function = std::function<F>;

}