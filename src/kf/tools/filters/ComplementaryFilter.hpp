#pragma once

#include "kf/units.hpp"

namespace kf {
namespace tools {

template<typename T> struct ComplementaryFilter {

private:
    const f32 alpha;
    const f32 one_minus_alpha{1.0f - alpha};
    T filtered{};
    bool first_step{true};

public:
    explicit ComplementaryFilter(f32 alpha) :
        alpha{alpha} {}

    const T &calc(T x, T dx, Seconds dt) {
        if (first_step) {
            first_step = false;
            filtered = x;
        } else {
            T prediction = filtered + dx * dt;
            filtered = alpha * prediction + one_minus_alpha * x;
        }

        return filtered;
    }

    void reset() {
        first_step = true;
    }
};

}// namespace tools
}// namespace kf
