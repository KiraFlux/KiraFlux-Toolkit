#include <kf/String.hpp>
#include <kf/main.hpp>

#include <kf/Slice.hpp>

using kf::Slice;

void kf::main(kf::Init &init) {
    char buffer[128];
    kf::String str{{buffer}};

    init.logger.debug("=== Slice Demo ===");

    int raw[] = {10, 20, 30, 40, 50};
    Slice<int> s{raw, 5}; // from pointer+size
    Slice<int> s_arr{raw};// from array (deduced size)

    for (auto i = 0; i < s.length(); i += 1) {
        str.format("s[{}] = {}", i, s[i]);
        init.logger.debug(str.view());
    }

    s[2] = 99;// modification

    // iteration

    str.reset();
    str.append("elements: ");
    for (auto x: s) {
        str.append(x);
        str.append(' ');
    }
    init.logger.debug(str.view());

    // sub‑slices
    auto first = s.first(2);
    auto last = s.last(2);
    auto mid = s.sub(1, kf::someTrivial<kf::usize>(3));
    auto from2 = s.fromOffset(2);

    auto print = [&](auto slice, const char *tag) {
        str.reset();

        str.append(tag);
        str.append(": ");

        for (auto x: slice) {
            str.append(x);
            str.append(' ');
        }

        init.logger.debug(str.view());
    };

    print(first, "first(2)");
    print(last, "last(2)");
    print(mid, "sub(1,3)");
    print(from2, "fromOffset(2)");

    // const‑correctness
    const int carr[] = {7, 8, 9};
    Slice<const int> cs{carr, 3};// const slice
    // cs[1] = 0; // error
    print(cs, "const");

    // conversion to const slice
    Slice<const int> cs2 = s;// operator Slice<const T>
    print(cs2, "converted");

    // empty slice
    Slice<int> empty{raw, 0};
    // empty.length(); // 0
}
