#pragma once

#include "kf/port/autoconfig.hpp"

#include "kf/attributes.hpp"
#include "kf/deque.hpp"

#if kf_port_has_queue

#include <queue>

namespace kf {
template<typename T, typename Container = kf::deque<T>> using queue = std::queue<T, Container>;
}

#else

namespace kf {

template<typename T, typename Container = kf::deque<T>> class queue {
private:
    Container c;

public:
    using value_type = typename Container::value_type;
    using container_type = Container;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;

    queue() = default;

    reference front() { return c.front(); }
    const_reference front() const { return c.front(); }

    reference back() { return c.back(); }
    const_reference back() const { return c.back(); }

    kf_nodiscard bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }

    // Модификация
    void push(const value_type &value) { c.push_back(value); }

    void push(value_type &&value) { c.push_back(kf::move(value)); }

    void pop() { c.pop_front(); }

    void swap(queue &other) noexcept {
        Container temp = kf::move(c);
        c = kf::move(other.c);
        other.c = kf::move(temp);
    }
};

}// namespace kf

#endif
