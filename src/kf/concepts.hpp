// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>

namespace kf {

template<typename Impl, typename Tag>
concept implements = std::derived_from<Impl, Tag>;

}// namespace kf