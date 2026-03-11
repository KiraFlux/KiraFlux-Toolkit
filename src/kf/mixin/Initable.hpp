#pragma once

#include <type_traits>

#include "kf/meta/CRTP.hpp"

namespace kf::mixin {

struct InitableTag {};

template<typename Impl, typename T, typename = void> struct Initable : InitableTag {
    [[nodiscard]] T init() noexcept {
        return static_cast<Impl *>(this)->initImpl();
    }
};

template<typename Impl> struct Initable<Impl, void> : InitableTag {
    void init() noexcept {
        static_cast<Impl *>(this)->initImpl();
    }
};

}// namespace kf::mixin
