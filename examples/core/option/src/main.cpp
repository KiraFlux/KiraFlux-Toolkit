// KiraFlux-Toolkit Example 'core/option'

#include <kf/Option.hpp>
#include <kf/main.hpp>

#include <kf/Function.hpp>
#include <kf/Slice.hpp>
#include <kf/mixin/ReprTo.hpp>

struct Point : kf::mixin::ReprTo<Point> {
    int x, y;

private:
    KF_IMPL_REPR_TO(Point);
    constexpr void reprToImpl(kf::implements<kf::mixin::WritableTag<char>> auto &w) const {
        w.format("Point({}, {})", x, y);
    }
};

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/option");

    // --- Basics: Some and None ---

    Option<int> opt_int = some(42);
    Option<int> opt_empty = none;

    init.logger.debug("opt_int: {}", opt_int);    // some(42)
    init.logger.debug("opt_empty: {}", opt_empty);// none

    // --- unwrapOr() ---

    init.logger.debug("opt_int.unwrapOr(0): {}", opt_int.unwrapOr(0));
    init.logger.debug("opt_empty.unwrapOr(100): {}", opt_empty.unwrapOr(100));

    // --- Checked extraction with unwrap() ---
    if (opt_int.isSome()) {
        init.logger.debug("opt_int.unwrap(): {}", opt_int.unwrap());
    }
    if (opt_empty.isNone()) {
        init.logger.debug("opt_empty is None, skipping unwrap");
    }

    // --- map() ---

    auto mapped = opt_int.map([](int x) { return x * 2; });
    init.logger.debug("opt_int.map(*2): {}", mapped);// some(84)

    auto void_mapped = opt_int.map([&init](int x) {
        init.logger.debug("processing value: {}", x);
    });
    init.logger.debug("void_mapped: {}", void_mapped);// some()

    auto mapped_empty = opt_empty.map([](int x) { return x * 2; });
    init.logger.debug("mapped_empty: {}", mapped_empty);// none

    // --- Option<Function<...>> ---

    Function<int(int)> add_one = [](int x) { return x + 1; };
    Option<Function<int(int)>> opt_func = some(std::move(add_one));

    if (opt_func.isSome()) {
        init.logger.debug("opt_func(41): {}", opt_func.unwrap()(41));
    }

    // --- Option<T&> ---

    int answer = 42;
    Option<int &> ref_opt = someRef(answer);

    if (ref_opt.isSome()) {
        ref_opt.unwrap() = 100;
        init.logger.debug("answer changed via ref: {}", answer);
    }
    init.logger.debug("ref_opt: {}", ref_opt);// some(100)

    // --- Sentinel optimisation for trivial types ---

    Option<usize> opt_usize = some(usize{100});
    Option<usize> opt_usize_empty = none;
    init.logger.debug("usize: {}, empty: {}", opt_usize, opt_usize_empty);

    Option<float> opt_float = some(3.14f);
    Option<float> opt_float_empty = none;
    init.logger.debug("float: {}, empty: {}", opt_float, opt_float_empty);

    enum class Color {
        Red,
        Green,
        Blue
    };
    Option<Color> opt_color = some(Color::Green);
    Option<Color> opt_color_empty = none;
    init.logger.debug("enum: {}, empty: {}", opt_color, opt_color_empty);

    // --- Trivial copyability ---

    static_assert(std::is_trivially_copyable_v<Option<int>>);
    static_assert(std::is_trivially_copyable_v<Option<float>>);
    static_assert(std::is_trivially_copyable_v<Option<Color>>);
    static_assert(std::is_trivially_copyable_v<Option<usize>>);
    init.logger.info("All static_asserts passed.");

    // --- Reset ---

    Option<int> reset_me = some(123);
    init.logger.debug("before reset: {}", reset_me);
    reset_me.reset();
    init.logger.debug("after reset: {}", reset_me);

    // --- Custom type with ReprTo ---

    Option<Point> opt_point = some(Point{.x = 10, .y = 20});
    Option<Point> opt_point_empty = none;
    init.logger.debug("opt_point: {}", opt_point);            // some((10,20))
    init.logger.debug("opt_point_empty: {}", opt_point_empty);// none
}