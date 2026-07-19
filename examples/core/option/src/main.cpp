// KiraFlux-Toolkit Example 'core/option'

#include <kf/Function.hpp>
#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/main.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/option");

    // --- Option<T> for nullable values ---

    // Option<T> is used throughout the library to represent values that may be absent.
    // It is a safer alternative to null pointers or sentinel values.
    // Use some(value) to create an Option containing a value, and none for empty.

    Option<int> opt_int = some(42);
    Option<int> opt_empty = none;

    init.logger.info("opt_int isSome: {}, isNone: {}", opt_int.isSome(), opt_int.isNone());
    init.logger.info("opt_empty isSome: {}, isNone: {}", opt_empty.isSome(), opt_empty.isNone());

    // --- Safe extraction with unwrapOr() ---

    // unwrapOr() returns the contained value if Some, otherwise a default.
    // This is the preferred way to extract a value when a fallback is acceptable.
    // Use it when the absence of a value has a sensible default.

    int value1 = opt_int.unwrapOr(0);
    int value2 = opt_empty.unwrapOr(100);

    init.logger.info("opt_int.unwrapOr(0) = {}", value1);
    init.logger.info("opt_empty.unwrapOr(100) = {}", value2);

    // --- Checked extraction with isSome() ---

    // unwrap() aborts if the Option is None – use it only when you are certain
    // the value exists. Always check isSome() first in user code.
    // Many library internals use unwrap() after internal validation.

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
    // Option<void> is not meant to be used as a flag – it exists for completeness
    // of the map() API to support functions that return void.

    auto void_mapped = opt_int.map([&](int x) {
        init.logger.debug("processing value: {}", x);
    });

    init.logger.info("void_mapped isSome: {}", void_mapped.isSome());

    // map on None yields None (the function is never called).
    auto mapped_empty = opt_empty.map([](int x) {
        return x * 2;
    });

    init.logger.info("mapped_empty isSome: {}", mapped_empty.isSome());

    // --- Option<Slice<T>> for optional buffers ---

    // Option<Slice<T>> is used when a function may or may not return a buffer view.
    // For example, a parser might return a Slice pointing into the input if a token
    // is found, or None if not. This avoids returning empty slices that are ambiguous.

    int data[] = {10, 20, 30};
    Slice<int> slice{data};

    Option<Slice<int>> opt_slice = some(slice);
    Option<Slice<int>> opt_slice_empty = none;

    if (opt_slice.isSome()) {
        auto s = opt_slice.unwrap();
        init.logger.info("slice length: {}", s.length());
        for (int v: s) {
            init.logger.debug("  {}", v);
        }
    }

    // opt_slice_empty is None – safe to handle with isSome().

    // --- Option<Function<...>> for optional callbacks ---

    // kf::Function is a type-erased callable that is never empty.
    // To represent an optional callback (e.g., an event handler that may not be set),
    // wrap it in Option<Function<...>>. Use some() to set a callback, none to clear it.
    // Many UI widgets and listeners use this pattern.

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

    // --- TrivialOption for performance-critical paths ---

    // TrivialOption<T> is a memory-optimized version for trivially copyable types.
    // It uses a sentinel value (e.g., NaN for floats, max value for usize) to indicate None.
    // Use it in hot paths where Option<T> would add overhead (e.g., in math filters,
    // PID controllers, or any high-frequency processing).

    TrivialOption<float> tr_float = someTrivial(3.14f);
    TrivialOption<float> tr_float_empty = none;

    init.logger.info("tr_float: {}, tr_float_empty: {}", tr_float.unwrapOr(0.0f), tr_float_empty.unwrapOr(0.0f));

    // TrivialOption<usize> also works – uses (usize)-1 as the sentinel.
    TrivialOption<usize> tr_usize = someTrivial(usize{100});
    TrivialOption<usize> tr_usize_empty = none;

    init.logger.info("tr_usize: {}, tr_usize_empty: {}", tr_usize.unwrapOr(0), tr_usize_empty.unwrapOr(0));

    // --- Resetting an Option ---

    // reset() discards the contained value and turns the Option into None.
    // This is useful when you want to explicitly clear a stored value.

    Option<int> reset_me = some(123);
    reset_me.reset();
    init.logger.info("after reset: isSome = {}", reset_me.isSome());
}