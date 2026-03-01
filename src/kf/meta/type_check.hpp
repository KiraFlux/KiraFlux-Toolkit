// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#define kf_crtp_check(__impl, __tag) static_assert(std::is_base_of_v<__tag, __impl>, "'" #__impl "' must inherit from '" #__tag "'")