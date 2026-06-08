#include <kf/Slice.hpp>
#include <unity.h>

using kf::Slice;

namespace constructors {
void default_ctor() {
    Slice<int> s{};
    TEST_ASSERT_NULL(s.data());
    TEST_ASSERT_EQUAL(0, s.size());
    TEST_ASSERT_TRUE(s.empty());
}

void ptr_size_ctor() {
    int data[] = {1, 2, 3};
    Slice<int> s{data, 3};
    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(3, s.size());
    TEST_ASSERT_FALSE(s.empty());
}

void array_int() {
    int data[] = {1, 2, 3, 4, 5};
    Slice<int> s{data};
    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(5, s.size());
    TEST_ASSERT_EQUAL(1, s[0]);
    TEST_ASSERT_EQUAL(3, s[2]);
    TEST_ASSERT_EQUAL(5, s[4]);
}

void array_char() {
    char data[] = "hello";
    Slice<char> s{data};
    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(6, s.size());
    TEST_ASSERT_EQUAL('h', s[0]);
    TEST_ASSERT_EQUAL('e', s[1]);
    TEST_ASSERT_EQUAL('l', s[2]);
    TEST_ASSERT_EQUAL('l', s[3]);
    TEST_ASSERT_EQUAL('o', s[4]);
}

void array_const() {
    const int data[] = {10, 20, 30};
    Slice<const int> s{data};
    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(3, s.size());
    TEST_ASSERT_EQUAL(10, s[0]);
    TEST_ASSERT_EQUAL(20, s[1]);
    TEST_ASSERT_EQUAL(30, s[2]);
}

void array_modify() {
    int data[] = {7, 8, 9};
    Slice<int> s{data};
    s[1] = 99;
    TEST_ASSERT_EQUAL(99, data[1]);
    TEST_ASSERT_EQUAL(99, s[1]);
}

void array_empty() {
    int data[1] = {42};
    Slice<int> s{data};
    TEST_ASSERT_EQUAL(1, s.size());
    TEST_ASSERT_EQUAL(42, s[0]);

    int *nullPtr = nullptr;
    Slice<int> s2{nullPtr, 0};
    TEST_ASSERT_NULL(s2.data());
    TEST_ASSERT_EQUAL(0, s2.size());
}
}// namespace constructors

namespace access {
void element() {
    int data[] = {10, 20, 30};
    Slice<int> s{data, 3};
    TEST_ASSERT_EQUAL(10, s[0]);
    TEST_ASSERT_EQUAL(20, s[1]);
    TEST_ASSERT_EQUAL(30, s[2]);
    s[1] = 99;
    TEST_ASSERT_EQUAL(99, data[1]);
}

void const_slice() {
    const int data[] = {7, 8, 9};
    Slice<const int> s{data, 3};
    TEST_ASSERT_EQUAL(7, s[0]);
    TEST_ASSERT_EQUAL(8, s[1]);
    TEST_ASSERT_EQUAL(9, s[2]);
}

void iterators() {
    int data[] = {5, 6, 7, 8};
    Slice<int> s{data, 4};

    int sum = 0;
    for (auto it = s.begin(); it != s.end(); it += 1) {
        sum += *it;
    }
    TEST_ASSERT_EQUAL(5 + 6 + 7 + 8, sum);

    sum = 0;
    for (auto v: s) {
        sum += v;
    }
    TEST_ASSERT_EQUAL(5 + 6 + 7 + 8, sum);
}
}// namespace access

namespace sub_slice {
void sub() {
    int data[] = {1, 2, 3, 4, 5, 6};
    Slice<int> s{data, 6};
    auto sub = s.sub(2, 3);
    TEST_ASSERT_EQUAL_PTR(data + 2, sub.data());
    TEST_ASSERT_EQUAL(3, sub.size());
    TEST_ASSERT_EQUAL(3, sub[0]);
    TEST_ASSERT_EQUAL(4, sub[1]);
    TEST_ASSERT_EQUAL(5, sub[2]);
}

void first() {
    int data[] = {1, 2, 3, 4};
    Slice<int> s{data, 4};
    auto f = s.first(2);
    TEST_ASSERT_EQUAL_PTR(data, f.data());
    TEST_ASSERT_EQUAL(2, f.size());
    TEST_ASSERT_EQUAL(1, f[0]);
    TEST_ASSERT_EQUAL(2, f[1]);
}

void last() {
    int data[] = {1, 2, 3, 4};
    Slice<int> s{data, 4};
    auto l = s.last(2);
    TEST_ASSERT_EQUAL_PTR(data + 2, l.data());
    TEST_ASSERT_EQUAL(2, l.size());
    TEST_ASSERT_EQUAL(3, l[0]);
    TEST_ASSERT_EQUAL(4, l[1]);
}

void from_offset() {
    int data[] = {1, 2, 3, 4, 5};
    Slice<int> s{data, 5};
    auto off = s.fromOffset(2);
    TEST_ASSERT_EQUAL_PTR(data + 2, off.data());
    TEST_ASSERT_EQUAL(3, off.size());
    TEST_ASSERT_EQUAL(3, off[0]);
    TEST_ASSERT_EQUAL(4, off[1]);
    TEST_ASSERT_EQUAL(5, off[2]);
}
}// namespace sub_slice

namespace conversion {
void to_const() {
    int data[] = {1, 2, 3};
    Slice<int> s{data, 3};
    Slice<const int> cs = s;
    TEST_ASSERT_EQUAL_PTR(s.data(), cs.data());
    TEST_ASSERT_EQUAL(s.size(), cs.size());
    TEST_ASSERT_EQUAL(1, cs[0]);
    TEST_ASSERT_EQUAL(2, cs[1]);
    TEST_ASSERT_EQUAL(3, cs[2]);
}

void empty_to_const() {
    Slice<int> s{};
    Slice<const int> cs = s;
    TEST_ASSERT_NULL(cs.data());
    TEST_ASSERT_EQUAL(0, cs.size());
    TEST_ASSERT_TRUE(cs.empty());
}

void const_slice_from_const_ptr() {
    const int data[] = {4, 5, 6};
    Slice<const int> s{data, 3};
    Slice<const int> cs = s;
    TEST_ASSERT_EQUAL_PTR(s.data(), cs.data());
    TEST_ASSERT_EQUAL(s.size(), cs.size());
    TEST_ASSERT_EQUAL(4, cs[0]);
    TEST_ASSERT_EQUAL(5, cs[1]);
    TEST_ASSERT_EQUAL(6, cs[2]);
}
}// namespace conversion

namespace misc {
void empty() {
    int data[] = {1, 2, 3};
    Slice<int> s{data, 0};
    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(0, s.size());
    TEST_ASSERT_TRUE(s.empty());

    Slice<int> s2{};
    TEST_ASSERT_NULL(s2.data());
    TEST_ASSERT_EQUAL(0, s2.size());

    auto sub = s.sub(0, 0);
    TEST_ASSERT_EQUAL_PTR(data, sub.data());
    TEST_ASSERT_EQUAL(0, sub.size());
}

void copy_assign() {
    int data[] = {1, 2, 3};
    Slice<int> a{data, 3};
    Slice<int> b = a;
    TEST_ASSERT_EQUAL_PTR(a.data(), b.data());
    TEST_ASSERT_EQUAL(a.size(), b.size());

    int other[] = {4, 5};
    b = Slice<int>{other, 2};
    TEST_ASSERT_EQUAL_PTR(other, b.data());
    TEST_ASSERT_EQUAL(2, b.size());
}

void different_types() {
    char cdata[] = "abc";
    Slice<char> cs{cdata, 3};
    TEST_ASSERT_EQUAL('a', cs[0]);

    double ddata[] = {1.1, 2.2};
    Slice<double> ds{ddata, 2};
    TEST_ASSERT_EQUAL_FLOAT(1.1, ds[0]);
}
}// namespace misc

int main() {
    UNITY_BEGIN();

    RUN_TEST(constructors::default_ctor);
    RUN_TEST(constructors::ptr_size_ctor);
    RUN_TEST(constructors::array_int);
    RUN_TEST(constructors::array_char);
    RUN_TEST(constructors::array_const);
    RUN_TEST(constructors::array_modify);
    RUN_TEST(constructors::array_empty);

    RUN_TEST(access::element);
    RUN_TEST(access::const_slice);
    RUN_TEST(access::iterators);

    RUN_TEST(sub_slice::sub);
    RUN_TEST(sub_slice::first);
    RUN_TEST(sub_slice::last);
    RUN_TEST(sub_slice::from_offset);

    RUN_TEST(conversion::to_const);
    RUN_TEST(conversion::empty_to_const);
    RUN_TEST(conversion::const_slice_from_const_ptr);

    RUN_TEST(misc::empty);
    RUN_TEST(misc::copy_assign);
    RUN_TEST(misc::different_types);

    return UNITY_END();
}