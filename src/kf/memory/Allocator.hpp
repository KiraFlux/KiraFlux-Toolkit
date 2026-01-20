// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/config.hpp"

#if kf_port_has_memory
#include <memory>

namespace kf {

template<typename T> using Allocator = std::allocator<T>;

}// namespace kf

#else

#include "kf/core/aliases.hpp"
#include "kf/core/type_traits.hpp"
#include "kf/core/utility.hpp"

/// @brief Placement new operator for AVR/embedded platforms
/// @param size Unused parameter (required by C++ standard)
/// @param ptr Memory location to construct object at
/// @return Pointer to constructed object
inline void *operator new(kf::usize, void *ptr) noexcept {
    return ptr;
}

/// @brief Placement delete operator for AVR/embedded platforms
/// @param ptr Unused pointer parameter
/// @param place Unused placement parameter
inline void operator delete(void *, void *) noexcept {}

namespace kf {

/// @brief Custom allocator for embedded systems without standard library
/// @tparam T Type of objects to allocate
/// @note Provides basic dynamic memory allocation for constrained environments
template<typename T> class Allocator {
public:
    using value_type = T;       ///< Type of allocated elements
    using pointer = T *;        ///< Pointer to allocated element
    using size_type = kf::usize;///< Size type for allocation counts

    /// @brief Allocate raw memory for n objects of type T
    /// @param n Number of objects to allocate space for
    /// @return Pointer to allocated memory block
    /// @note Memory is uninitialized (no constructors called)
    pointer allocate(size_type n) {
        return static_cast<pointer>(::operator new(n * sizeof(T)));
    }

    /// @brief Deallocate previously allocated memory
    /// @param p Pointer to memory block to deallocate
    /// @param size_type Unused parameter (required by allocator interface)
    void deallocate(pointer p, size_type) {
        ::operator delete(p);
    }

    /// @brief Construct object in pre-allocated memory (placement new)
    /// @tparam U Type of object to construct
    /// @tparam Args Types of constructor arguments
    /// @param p Pointer to memory location for construction
    /// @param args Arguments to forward to constructor
    template<typename U, typename... Args> void construct(U *p, Args &&...args) {
        ::new (static_cast<void *>(p)) U(forward<Args>(args)...);
    }

    /// @brief Destroy object without deallocating memory
    /// @tparam U Type of object to destroy
    /// @param p Pointer to object to destroy
    template<typename U> void destroy(U *p) {
        p->~U();
    }
};

}// namespace kf

#endif