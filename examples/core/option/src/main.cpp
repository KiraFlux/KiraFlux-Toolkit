// KiraFlux-Toolkit Example 'core/option'

#include <kf/Function.hpp>
#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/main.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/option");

    // --- Basics: Some and None ---

    // Option<T> represents an optional value – either Some(T) or None.
    // Use some(value) to create an Option with a value, and none for empty.
    // This is a safer alternative to null pointers or sentinel values.

    Option<int> opt_int = some(42);
    Option<int> opt_empty = none;

    init.logger.info("opt_int: isSome={}, isNone={}", opt_int.isSome(), opt_int.isNone());
    init.logger.info("opt_empty: isSome={}, isNone={}", opt_empty.isSome(), opt_empty.isNone());

    // --- Safe extraction with unwrapOr() ---

    // unwrapOr() returns the contained value if Some, otherwise the provided default.
    // It is the preferred way to extract a value when a fallback is acceptable.

    int value1 = opt_int.unwrapOr(0);
    int value2 = opt_empty.unwrapOr(100);

    init.logger.info("opt_int.unwrapOr(0) = {}", value1);
    init.logger.info("opt_empty.unwrapOr(100) = {}", value2);

    // --- Checked extraction with isSome() ---

    // unwrap() aborts if the Option is None – use it only when you are certain
    // the value exists, or after an isSome() check.
    // Library internals often use unwrap() after internal validation.

    if (opt_int.isSome()) {
        init.logger.info("opt_int.unwrap() = {}", opt_int.unwrap());
    }

    if (opt_empty.isNone()) {
        init.logger.info("opt_empty is None, skipping unwrap()");
    }

    // --- Transformation with map() ---

    // map() applies a function to the contained value if Some, returning a new Option.
    // If the Option is None, map() returns None without calling the function.
    // This is useful for chaining transformations without explicit checks.

    auto mapped = opt_int.map([](int x) {
        return x * 2;
    });

    if (mapped.isSome()) {
        init.logger.info("mapped: {}", mapped.unwrap());
    }

    // When the mapping function returns void, map() produces Option<void>.
    // This is useful for side-effects that should only run when a value is present.
    // Option<void> exists for completeness of the map() API; it is not meant as a flag.

    auto void_mapped = opt_int.map([&](int x) {
        init.logger.debug("processing value: {}", x);
    });

    init.logger.info("void_mapped isSome: {}", void_mapped.isSome());

    // map on None yields None (the function is never called).
    auto mapped_empty = opt_empty.map([](int x) {
        return x * 2;
    });

    init.logger.info("mapped_empty isSome: {}", mapped_empty.isSome());

    // --- Option<Function<...>> for optional callbacks ---

    // kf::Function is a type-erased callable that is never empty.
    // To represent an optional callback (e.g., an event handler that may not be set),
    // wrap it in Option<Function<...>>. Use some() to set a callback, none to clear it.
    // This pattern is common in UI widgets and listeners.

    Function<int(int)> add_one = [](int x) { return x + 1; };
    Option<Function<int(int)>> opt_func = some(std::move(add_one));

    if (opt_func.isSome()) {
        int result = opt_func.unwrap()(41);
        init.logger.info("opt_func(41) = {}", result);
    }

    Option<Function<int(int)>> opt_func_empty = none;
    init.logger.info("opt_func_empty isSome: {}", opt_func_empty.isSome());

    // --- Option<T&> for optional references ---

    // Option<T&> stores a reference to an existing object, or None.
    // It is created with someRef() and is useful for returning references to
    // container elements that may or may not exist (e.g., front() of an empty queue).
    // The referenced object must outlive the Option.

    int answer = 42;
    Option<int &> ref_opt = someRef(answer);

    if (ref_opt.isSome()) {
        ref_opt.unwrap() = 100;// modifies the original variable
        init.logger.info("answer changed via ref: {}", answer);
    }

    // --- Sentinel optimisation for trivial types ---

    // For trivially copyable types that have a dedicated sentinel value, Option<T>
    // uses SentinelStorage which eliminates the separate boolean flag.
    // The following types use sentinel values:

    // 1) usize: uses (usize)-1 as None.
    Option<usize> opt_usize = some(usize{100});
    Option<usize> opt_usize_empty = none;
    init.logger.info("usize: {}, empty: {}", opt_usize.unwrapOr(0), opt_usize_empty.unwrapOr(0));

    // 2) float: uses quiet_NaN as None.
    Option<float> opt_float = some(3.14f);
    Option<float> opt_float_empty = none;
    init.logger.info("float: {}, empty: {}", opt_float.unwrapOr(0.0f), opt_float_empty.unwrapOr(0.0f));

    // 3) enum: uses static_cast<Underlying>(-1) as None.
    enum class Color {
        Red,
        Green,
        Blue,
    };

    Option<Color> opt_color = some(Color::Green);
    Option<Color> opt_color_empty = none;
    init.logger.info("enum: {}, empty: {}",
                     static_cast<int>(opt_color.unwrapOr(Color::Red)),
                     static_cast<int>(opt_color_empty.unwrapOr(Color::Red)));

    // 4) Function: uses an empty Function (isSome() == false) as None.
    // Already shown above.

    // 5) References: uses nullptr as None.
    // Already shown above.

    // --- Trivial capability ---

    // Since SentinelStorage and TrivialStorage store only the value (or value + bool)
    // and are trivially copyable, Option<T> is trivially copyable for T that are
    // trivially copyable. This allows usage in tight loops and embedded contexts.

    static_assert(std::is_trivially_copyable_v<Option<int>>);
    static_assert(std::is_trivially_copyable_v<Option<float>>);
    static_assert(std::is_trivially_copyable_v<Option<Color>>);
    static_assert(std::is_trivially_copyable_v<Option<usize>>);
    // Option<Function<...>> is not trivially copyable because Function is not.

    init.logger.info("All static_asserts passed.");

    // --- Resetting an Option ---

    // reset() discards the contained value and turns the Option into None.
    // This is useful when you want to explicitly clear a stored value.

    Option<int> reset_me = some(123);
    reset_me.reset();
    init.logger.info("after reset: isSome = {}", reset_me.isSome());
}