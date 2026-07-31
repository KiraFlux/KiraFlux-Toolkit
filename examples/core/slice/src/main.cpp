// KiraFlux-Toolkit Example 'core/slice'

#include <kf/Slice.hpp>
#include <kf/main.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/slice");

    // --- Create slice from array ---

    int raw[] = {10, 20, 30, 40, 50};
    Slice<int> s{raw};// array constructor (deduced size 5)

    init.logger.debug("length: {}", s.length());

    // --- Access and modify ---

    for (usize i = 0; i < s.length(); i += 1) {
        init.logger.debug("s[{}] = {}", i, s[i]);
    }

    s[2] = 99;// modification

    // --- Iteration ---

    init.logger.debug("elements:");
    for (int x: s) {
        init.logger.debug("  {}", x);
    }

    // --- Sub-slices ---

    auto first = s.first(2);
    auto last = s.last(2);
    auto mid = s.sub(1, some<usize>(3));
    auto from2 = s.fromOffset(2);

    auto print_slice = [&](auto slice, char const *tag) {
        init.logger.debug("{}:", tag);
        for (auto x: slice) {
            init.logger.debug("  {}", x);
        }
    };

    print_slice(first, "first(2)");
    print_slice(last, "last(2)");
    print_slice(mid, "sub(1,3)");
    print_slice(from2, "fromOffset(2)");

    // --- Const slice ---

    int const carr[] = {7, 8, 9};
    Slice<int const> cs{carr};
    print_slice(cs, "const");

    // --- Implicit conversion to const ---

    Slice<int const> cs2 = s;
    print_slice(cs2, "converted");

    // --- Empty slice ---

    Slice<int> empty{raw, 0};
    init.logger.debug("empty length: {}", empty.length());
    init.logger.debug("empty is empty: {}", empty.empty());
}