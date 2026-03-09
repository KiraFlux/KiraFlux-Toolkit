// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/memory/io/tags.hpp"
#include "kf/meta/type_check.hpp"

#include "kf/bus/Tag.hpp"

namespace kf::bus {

/// @brief Bus static interface
/// @tparam BusImpl Bus implementation.
/// @note CRTP base class for bus implementations. Template parameter BusImpl must provide:
///       - an `Error` type,
///       - a nested `Node` type that inherits from `memory::io::Readable` and `memory::io::Writable` (checked via `kf_crtp_check`),
///       - a nested `Node::Config` type,
///       - a constructor `Node(BusImpl&, const Node::Config&)` for creating device nodes.
///       The bus does not manage node lifetimes; it only offers a factory.
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
