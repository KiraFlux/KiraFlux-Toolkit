// KiraFlux-Toolkit Example 'core/slice'

#include <kf/Slice.hpp>
#include <kf/main.hpp>

#include <kf/Bytes.hpp>
#include <kf/BytesView.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/slice");

    // --- 1. Construct from array (Sequence feature) ---
    int raw[] = {10, 20, 30, 40, 50};
    Slice<int> s{raw};// array constructor, length deduced

    init.logger.debug("length: {}", s.length());// Sequence::length()
    init.logger.debug("full slice: {}", s);     // ReprTo: {10, 20, 30, 40, 50}

    // --- 2. Indexing and modification (Sequence::operator[]) ---
    for (usize i = 0; i < s.length(); ++i) {
        init.logger.debug("s[{}] = {}", i, s[i]);
    }

    s[2] = 99;                                     // modify via Sequence::operator[]
    init.logger.debug("after modification: {}", s);// {10, 20, 99, 40, 50}

    // --- 3. Iteration (Sequence provides begin/end) ---
    init.logger.debug("iterating over slice:");
    for (int v: s) {
        init.logger.debug("  {}", v);
    }

    // --- 4. Slicing (Slice-specific methods) ---
    auto first = s.first(2);
    auto last = s.last(2);
    auto mid = s.sub(1, some<usize>(3));
    auto from2 = s.fromOffset(2);

    init.logger.debug("first(2): {}", first);     // {10, 20}
    init.logger.debug("last(2): {}", last);       // {40, 50}
    init.logger.debug("sub(1,3): {}", mid);       // {20, 99, 40}
    init.logger.debug("fromOffset(2): {}", from2);// {99, 40, 50}

    // --- 5. Const slice (Sequence works with const T) ---
    int const carr[] = {7, 8, 9};
    Slice<int const> cs{carr};
    init.logger.debug("const slice: {}", cs);// {7, 8, 9}

    // --- 6. Implicit conversion to const (Slice<T> -> Slice<T const>) ---
    Slice<int const> cs2 = s;
    init.logger.debug("converted to const: {}", cs2);// {10, 20, 99, 40, 50}

    // --- 7. Empty slice (Sequence::empty() from length) ---
    Slice<int> empty{raw, 0};
    init.logger.debug("empty length: {}", empty.length());
    init.logger.debug("empty is empty: {}", empty.empty());// true

    // --- 8. Slice<char> treated as string (ReprTo special case) ---
    char text[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    Slice<char> str_slice{text, 11};
    init.logger.debug("char slice: {}", str_slice);// "Hello World" (no braces)

    char const ctext[] = "KiraFlux";
    Slice<char const> const_str{ctext, 8};
    init.logger.debug("const char slice: {}", const_str);// "KiraFlux"

    // --- 9. Slice<u8> (Bytes) is NOT treated as string ---
    u8 bytes[] = {0x01, 0x02, 0x03, 0xFF};
    Slice<u8> byte_slice{bytes, 4};
    init.logger.debug("u8 slice: {}", byte_slice);// {1, 2, 3, 255}

    BytesView bv{bytes, 4};
    init.logger.debug("BytesView: {}", bv);// {1, 2, 3, 255}
}