// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>


namespace kf {

using std::conditional;
using std::enable_if;
using std::false_type;
using std::integral_constant;
using std::is_arithmetic;
using std::is_base_of;
using std::is_const;
using std::is_convertible;
using std::is_floating_point;
using std::is_integral;
using std::is_null_pointer;
using std::is_pointer;
using std::is_reference;
using std::is_same;
using std::is_void;
using std::is_volatile;
using std::remove_const;
using std::remove_cv;
using std::remove_reference;
using std::remove_volatile;
using std::true_type;
using std::is_nothrow_constructible;
using std::is_nothrow_move_constructible;

}// namespace kf
