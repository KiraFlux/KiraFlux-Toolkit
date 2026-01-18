#pragma once

#include "kf/port/autoconfig.hpp"

#if defined(__cplusplus) && __cplusplus >= 201703L
#define kf_if_constexpr if constexpr
#else
#define kf_if_constexpr if
#endif

#if defined(__cplusplus) && __cplusplus >= 201703L
#define kf_nodiscard [[nodiscard]]
#else
#define kf_nodiscard
#endif

#if defined(__cplusplus) && __cplusplus >= 201703L
#define kf_maybe_unused [[maybe_unused]]
#else
#define kf_maybe_unused
#endif

#if defined(__cplusplus) && __cplusplus >= 201402L
#define kf_deprecated(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
#define kf_deprecated(msg) __attribute__((deprecated(msg)))
#else
#define kf_deprecated(msg)
#endif
