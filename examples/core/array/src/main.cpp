// KiraFlux-Toolkit Example 'core/array'

#include <kf/Array.hpp>
#include <kf/main.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/array");

    // --- Array overview ---

    // Array<T, N> is the ONLY owning container in the library.
    // It owns a fixed-size buffer of type T with N elements.
    // Memory is allocated statically or on the stack (no dynamic allocation).
    // Array inherits from Sequence, so it supports iteration, indexing, slicing,
    // and all Sequence operations (begin, end, data, slice, length, empty, operator[]).

    // --- Construction ---

    // Default construction: elements are default-initialized (for primitive types, uninitialized).
    kf::Array<int, 5> arr1;

    // Aggregate initialization with designated initializers (C++20).
    // You can initialize all or some elements; remaining are default-initialized.
    kf::Array<int, 5> arr2{.items = {1, 2, 3, 4, 5}};

    // Partial initialization: rest are zero-initialized (if using `{}`).
    kf::Array<int, 5> arr3{.items = {10, 20}};// arr3[2..4] = 0

    // --- Access and modification ---

    // operator[] (inherited from Sequence) provides random access.
    arr2[0] = 100;
    arr2[1] = 200;

    // Iteration (range-based for, inherited from Sequence).
    init.logger.debug("arr2 elements:");
    for (auto v: arr2) {
        init.logger.debug("  {}", v);
    }

    // --- Fill ---

    // fill() sets all elements to a given value.
    arr3.fill(42);
    init.logger.debug("arr3 after fill(42):");
    for (auto v: arr3) {
        init.logger.debug("  {}", v);
    }

    // --- Slice and data access ---

    // data() returns pointer to the underlying buffer (mutable and const).
    int *raw = arr2.data();
    const int *const_raw = arr2.data();// const overload

    // slice() returns a non-owning view of the entire array.
    auto slice = arr2.slice();
    init.logger.debug("slice length: {}", slice.length());

    // Since Slice is also a Sequence, you can iterate over it.
    for (auto v: slice) {
        // ...
    }

    // --- Length and capacity ---

    // length() is compile-time constant N (inherited from Sequence).
    static_assert(kf::Array<int, 5>::items_total == 5);
    init.logger.debug("arr2 length: {}", arr2.length());

    // empty() returns false if N > 0 (always false for fixed-size arrays).
    init.logger.debug("arr2 is empty: {}", arr2.empty());

    // --- Equality (mixin::Equatable) ---

    // Array is equatable if T is equatable.
    kf::Array<int, 3> a{.items = {1, 2, 3}};
    kf::Array<int, 3> b{.items = {1, 2, 3}};
    if (a == b) {
        init.logger.info("Arrays are equal");
    }

    // --- Use as buffer for Stack or Queue ---

    // Array is the primary way to provide memory for non-owning containers.

    // --- Performance note ---

    // All operations are O(1) and noexcept. No dynamic allocations.
    // Array is ideal for small, fixed-size collections in embedded systems.
}