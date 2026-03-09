// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/memory/io/tags.hpp"
#include "kf/meta/type_check.hpp"

#include "kf/bus/Tag.hpp"

namespace kf::bus {

/// @brief CRTP base class for bus implementations.
/// @tparam BusImpl Concrete bus implementation (must inherit from this class).
/// @tparam NodeImpl Type of node that will be created by the bus (must satisfy Readable and Writable).
/// @tparam ErrorImpl Error type used by bus operations.
/// @note The bus implementation must provide methods `initImpl()` and `quitImpl()`.
///       Nodes are created via `createNode` and are expected to be movable.
template<typename BusImpl, typename NodeImpl, typename ErrorImpl> struct Bus : bus::Tag {
    kf_crtp_check(NodeImpl, kf::memory::io::ReadableTag);
    kf_crtp_check(NodeImpl, kf::memory::io::WritableTag);

    [[nodiscard]] Result<void, ErrorImpl> init() noexcept { return impl().initImpl(); }

    void quit() noexcept { impl().quitImpl(); }

    [[nodiscard]] NodeImpl createNode(const typename NodeImpl::Config &config) noexcept { return NodeImpl{impl(), config}; }

private:
    BusImpl &impl() noexcept { return *static_cast<BusImpl *>(this); }
    const BusImpl &impl() const noexcept { return *static_cast<const BusImpl *>(this); }

    // methods:
    // ctor: BusImpl::Node(BusImpl &, Node::Config &)
    // [[nodiscard]] Result<void, Error> initImpl() noexcept
    // void quitImpl() noexcept
};

}// namespace kf::bus
