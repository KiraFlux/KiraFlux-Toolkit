// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/Readable.hpp"
#include "kf/mixin/Writable.hpp"

namespace kf::bus {

struct BusNodeTag {};

template<typename Impl, typename ErrorImpl> struct BusNode :

    BusNodeTag,
    mixin::NonCopyable,
    mixin::Readable<Impl, ErrorImpl>,
    mixin::Writable<Impl, Result<void, ErrorImpl>>

{};

struct BusTag {};

/// @brief CRTP base class for bus implementations.
/// @tparam BusImpl Concrete bus implementation (must inherit from this class).
/// @tparam NodeImpl Type of node that will be created by the bus (must satisfy Readable and Writable).
/// @tparam ErrorImpl Error type used by bus operations.
/// @note The bus implementation must provide methods `initImpl()` and `quitImpl()`.
///       Nodes are created via `createNode` and are expected to be movable.
template<typename BusImpl, typename NodeImpl, typename ErrorImpl> struct Bus :

    BusTag,
    mixin::NonCopyable,
    mixin::Initable<BusImpl, Result<void, ErrorImpl>()>,
    mixin::Quitable<BusImpl>,
    meta::CRTP<BusImpl>

{
    KF_CHECK_IMPL(NodeImpl, ::kf::bus::BusNodeTag);

    [[nodiscard]] NodeImpl createNode(const typename NodeImpl::Config &config) noexcept {
        return NodeImpl{this->impl(), config};
    }
};

}// namespace kf::bus
