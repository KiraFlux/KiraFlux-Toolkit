// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/BinaryReadable.hpp"
#include "kf/mixin/BinaryWritable.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"

namespace kf::bus {

struct BusNodeTag {};

template<typename Impl, typename ErrorImpl> struct BusNode :

    BusNodeTag,
    mixin::NonCopyable,
    mixin::BinaryReadable<Impl, ErrorImpl>,
    mixin::BinaryWritable<Impl, Result<void, ErrorImpl>>

{};

struct BusTag {};

/// @brief CRTP base class for bus implementations.
/// @tparam Impl Concrete bus implementation (must inherit from this class).
/// @tparam NodeImpl Type of node that will be created by the bus (must satisfy BinaryReadable and BinaryWritable).
/// @tparam ErrorImpl Error type used by bus operations.
/// @note The bus implementation must provide methods `initImpl()` and `quitImpl()`.
///       Nodes are created via `createNode` and are expected to be movable.
template<typename Impl, implements<BusNodeTag> NodeImpl, typename ErrorImpl> struct Bus :

    BusTag,
    mixin::NonCopyable,
    mixin::Initable<Impl, Result<void, ErrorImpl>()>,
    mixin::Quitable<Impl>

{
    [[nodiscard]] NodeImpl createNode(const typename NodeImpl::Config &config) noexcept {
        return NodeImpl{*static_cast<Impl *>(this), config};
    }
};

}// namespace kf::bus

#define KF_IMPL_BUS_NODE(__impl__, __error_impl__)     \
    KF_IMPL_BINARY_READABLE(__impl__, __error_impl__); \
    KF_IMPL_BINARY_WRITABLE(__impl__, ::kf::Result<void, __error_impl__>)

#define KF_IMPL_BUS(__impl__, __error_impl__)                         \
    KF_IMPL_INITABLE(__impl__, ::kf::Result<void, __error_impl__>()); \
    KF_IMPL_QUITABLE(__impl__)
