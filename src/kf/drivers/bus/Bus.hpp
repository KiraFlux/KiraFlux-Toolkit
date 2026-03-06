// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/memory/io/tags.hpp"
#include "kf/meta/type_check.hpp"

#include "kf/drivers/bus/Tag.hpp"

namespace kf::drivers::bus {

/// @brief Bus static interface
/// @tparam BusImpl Bus implementation.
template<typename BusImpl> struct Bus : Tag {
    using NodeImpl = typename BusImpl::Node;

    kf_crtp_check(NodeImpl, kf::memory::io::ReaderTag);
    kf_crtp_check(NodeImpl, kf::memory::io::WriterTag);

    [[nodiscard]] Result<void, typename BusImpl::Error> begin() noexcept { return impl().beginImpl(); }

    void end() noexcept { impl().endImpl(); }

    [[nodiscard]] Result<NodeImpl, typename BusImpl::Error> createNode(const NodeImpl::Config &config) noexcept { return NodeImpl{impl(), config}; }

private:
    BusImpl &impl() noexcept { return *static_cast<BusImpl *>(this); }
    const BusImpl &impl() const noexcept { return *static_cast<const BusImpl *>(this); }

    // BusImpl must provide:

    // types:
    // Error - enum with Bus operation Errors
    // Node: Writable, Readable
    // Node::Config

    // methods:
    // ctor: BusImpl::Node(BusImpl &, Node::Config &)
    // [[nodiscard]] Result<void, Error> beginImpl() noexcept
    // [[nodiscard]] Result<void, Error> endImpl() noexcept
};

}// namespace kf::drivers::bus
