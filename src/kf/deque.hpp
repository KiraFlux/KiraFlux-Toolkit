#pragma once

#include "kf/port/autoconfig.hpp"

#include "kf/allocator.hpp"
#include "kf/attributes.hpp"

#if kf_port_has_deque

#include <deque>

namespace kf {
template<typename T, typename Alloc = kf::allocator<T>> using deque = std::deque<T, Alloc>;
}

#else

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/type_traits.hpp"
#include "kf/utility.hpp"

namespace kf {

template<typename T, typename Alloc = allocator<T>> class deque {
private:
    T *buffer_ = nullptr;
    usize capacity_ = 0;
    usize size_ = 0;
    usize front_ = 0;
    Alloc alloc_;

public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = usize;
    using reference = value_type &;
    using const_reference = const value_type &;

    deque() = default;

    explicit deque(const allocator_type &alloc) :
        alloc_(alloc) {}

    ~deque() {
        clear();
        if (buffer_) {
            alloc_.deallocate(buffer_, capacity_);
        }
    }

    // Доступ к элементам
    reference operator[](size_type pos) {
        return buffer_[(front_ + pos) % capacity_];
    }

    const_reference operator[](size_type pos) const {
        return buffer_[(front_ + pos) % capacity_];
    }

    reference front() { return buffer_[front_]; }
    const_reference front() const { return buffer_[front_]; }

    reference back() { return buffer_[(front_ + size_ - 1) % capacity_]; }
    const_reference back() const { return buffer_[(front_ + size_ - 1) % capacity_]; }

    kf_nodiscard bool empty() const { return size_ == 0; }

    kf_nodiscard size_type size() const { return size_; }

    kf_nodiscard size_type capacity() const { return capacity_; }

    void push_back(const T &value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        usize pos = (front_ + size_) % capacity_;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + pos, value);
        ++size_;
    }

    void push_back(T &&value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        usize pos = (front_ + size_) % capacity_;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + pos, kf::move(value));
        ++size_;
    }

    void push_front(const T &value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        front_ = (front_ == 0) ? capacity_ - 1 : front_ - 1;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + front_, value);
        ++size_;
    }

    void push_front(T &&value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        front_ = (front_ == 0) ? capacity_ - 1 : front_ - 1;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + front_, kf::move(value));
        ++size_;
    }

    void pop_back() {
        if (size_ > 0) {
            usize pos = (front_ + size_ - 1) % capacity_;
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + pos);
            --size_;
        }
    }

    void pop_front() {
        if (size_ > 0) {
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + front_);
            front_ = (front_ + 1) % capacity_;
            --size_;
        }
    }

    void clear() {
        for (usize i = 0; i < size_; ++i) {
            usize pos = (front_ + i) % capacity_;
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + pos);
        }
        size_ = 0;
        front_ = 0;
    }

    void reserve(size_type new_capacity) {
        if (new_capacity <= capacity_) return;

        T *new_buffer = alloc_.allocate(new_capacity);

        for (usize i = 0; i < size_; ++i) {
            usize old_pos = (front_ + i) % capacity_;
            usize new_pos = i;
            allocator_traits<Alloc>::construct(alloc_, new_buffer + new_pos,
                                               kf::move(buffer_[old_pos]));
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + old_pos);
        }

        if (buffer_) {
            alloc_.deallocate(buffer_, capacity_);
        }

        buffer_ = new_buffer;
        capacity_ = new_capacity;
        front_ = 0;
    }

    deque(const deque &) = delete;
    deque &operator=(const deque &) = delete;
};

}// namespace kf

#endif
