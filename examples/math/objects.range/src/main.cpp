// KiraFlux-Toolkit Example 'math/objects.range'

#include <kf/main.hpp>
#include <kf/math.hpp>

using namespace kf::math;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: math/objects.range");

    // --- Construction with different types ---

    // Range works with any numeric type (int, float, double).
    Range<int> r_int = Range<int>::create(0, 100);
    Range<float> r_float = Range<float>::create(-1.0f, 1.0f);
    Range<double> r_double = Range<double>::create(-0.5, 0.5);

    // ReprTo prints as "[start..end]".
    init.logger.debug("int range: {}", r_int);
    init.logger.debug("float range: {}", r_float);
    init.logger.debug("double range: {}", r_double);

    // --- Length ---

    // length() returns end - start; may be negative if start > end (but typically start <= end).
    init.logger.debug("int length: {}", r_int.length());
    init.logger.debug("float length: {}", r_float.length());

    // --- Clamping ---

    // clamped() constrains any value to the interval (inclusive).
    // Works with any numeric type (auto conversion).
    auto clamped_int_inside = r_int.clamped(50);
    auto clamped_int_low = r_int.clamped(-10);
    auto clamped_int_high = r_int.clamped(200);

    init.logger.debug("clamped(50) = {}", clamped_int_inside);
    init.logger.debug("clamped(-10) = {}", clamped_int_low);
    init.logger.debug("clamped(200) = {}", clamped_int_high);

    // For float range, clamping works with double values too.
    auto clamped_float = r_float.clamped(0.5f);
    auto clamped_float_out = r_float.clamped(2.0f);
    init.logger.debug("clamped(0.5) = {}", clamped_float);
    init.logger.debug("clamped(2.0) = {}", clamped_float_out);

    // --- Direct instantiation (no factory) ---

    // You can also use aggregate initialization.
    Range<int> r_agg{.start = 10, .end = 20};
    init.logger.debug("aggregate range: {}", r_agg);

    // --- Use in configuration (example) ---

    // Range is often used in config structs; ReprTo makes logging easy.
    struct Config {
        Range<int> value_range;
        int step;
    };

    Config cfg{
        .value_range = Range<int>::create(0, 100),
        .step = 5,
    };
    init.logger.debug("config: range={}, step={}", cfg.value_range, cfg.step);
}