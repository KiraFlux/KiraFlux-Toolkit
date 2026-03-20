// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/memory/io/Readable.hpp"
#include "kf/memory/io/Writable.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"

namespace kf::bus {

struct BusNodeTag {};
template<typename Impl, typename ErrorImpl> struct BusNode : BusNodeTag, mixin::NonCopyable, memory::io::Readable<Impl, ErrorImpl>, memory::io::Writable<Impl, ErrorImpl> {};

struct BusTag {};

/// @brief CRTP base class for bus implementations.
/// @tparam BusImpl Concrete bus implementation (must inherit from this class).
/// @tparam NodeImpl Type of node that will be created by the bus (must satisfy Readable and Writable).
/// @tparam ErrorImpl Error type used by bus operations.
/// @note The bus implementation must provide methods `initImpl()` and `quitImpl()`.
///       Nodes are created via `createNode` and are expected to be movable.
template<typename BusImpl, typename NodeImpl, typename ErrorImpl>
struct Bus : BusTag,
             mixin::Initable<BusImpl, Result<void, ErrorImpl>>,
             mixin::NonCopyable,
             mixin::Quitable<BusImpl> {
    kf_crtp_check(NodeImpl, kf::bus::BusNodeTag);

    [[nodiscard]] NodeImpl createNode(const typename NodeImpl::Config &config) noexcept {
        return NodeImpl{*static_cast<BusImpl *>(this), config};
    }

    // methods:
    // ctor: BusImpl::Node(BusImpl &, Node::Config &)
    // [[nodiscard]] Result<void, Error> initImpl() noexcept
    // void quitImpl() noexcept
};

}// namespace kf::bus
