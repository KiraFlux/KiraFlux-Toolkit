#pragma once

#include "kf/aliases.hpp"
#include "kf/attributes.hpp"

namespace kf {

template<typename T> struct slice {

private:
    T *ptr_;
    usize size_;

public:
    slice() noexcept :
        ptr_{nullptr}, size_{0} {}

    slice(T *ptr, usize size) noexcept :
        ptr_{ptr}, size_{size} {}

    kf_nodiscard T *begin() noexcept { return ptr_; }

    kf_nodiscard T *end() noexcept { return ptr_ + size_; }

    // Data

    kf_nodiscard T *data() noexcept { return ptr_; }

    kf_nodiscard const T *data() const noexcept { return ptr_; }

    // Size

    kf_nodiscard usize size() const noexcept { return size_; }

    kf_nodiscard bool empty() const noexcept { return size_ == 0; }
};

}// namespace kf