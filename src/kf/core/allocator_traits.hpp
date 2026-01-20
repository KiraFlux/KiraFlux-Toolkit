// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/config.hpp"

#if kf_port_has_memory

#include <memory>

using std::allocator_traits;

#else

#include "kf/core/utility.hpp"

template<typename Alloc> struct allocator_traits {
    using allocator_type = Alloc;

    template<typename U> using rebind_alloc = allocator<U>;

    static typename Alloc::pointer allocate(Alloc &a, kf::usize n) {
        return a.allocate(n);
    }

    static void deallocate(Alloc &a, typename Alloc::pointer p, kf::usize n) {
        a.deallocate(p, n);
    }

    template<typename U, typename... Args> static void construct(Alloc &a, U *p, Args &&...args) {
        a.construct(p, kf::forward<Args>(args)...);
    }

    template<typename U> static void destroy(Alloc &a, U *p) {
        a.destroy(p);
    }
};

#endif