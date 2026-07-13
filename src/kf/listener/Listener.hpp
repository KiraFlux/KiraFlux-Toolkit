// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <kf/Option.hpp>
#include <kf/meta/CRTP.hpp>
#include <kf/mixin/Callbacked.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/Resettable.hpp>
#include <kf/mixin/TimedPollable.hpp>

namespace kf::listener {

struct ListenerTag {};

template<typename Impl, typename InputType, typename CallbackSignature> struct Listener :

    ListenerTag,
    meta::CRTP<Impl>,
    mixin::NonCopyable,
    mixin::Resettable<Impl>,
    mixin::TimedPollable<Impl>,
    mixin::Callbacked<CallbackSignature>

{

    /// @brief Set an actual value for listener
    /// @note this method should called before `poll`
    template<typename U> void set(U &&value) noexcept {
        _value = kf::some(std::forward<U>(value));
    }

protected:
    constexpr const Option<InputType> &value() const noexcept {
        return _value;
    }

private:
    Option<InputType> _value{none};
};

}// namespace kf::listener

#define KF_IMPL_LISTENER(__impl__) \
    KF_IMPL_RESETTABLE(__impl__);  \
    KF_IMPL_TIMED_POLLABLE(__impl__)
