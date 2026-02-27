#include <kf/memory/Slice.hpp>
#include <unity.h>

using kf::Slice;

void test_default_ctor() {
    Slice<int> s{};
    TEST_ASSERT_NULL(s.data());
    TEST_ASSERT_EQUAL(0, s.size());
    TEST_ASSERT_TRUE(s.empty());
}

void test_ptr_size_ctor() {
    int data[] = {1, 2, 3};
    Slice<int> s{data, 3};
    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(3, s.size());
    TEST_ASSERT_FALSE(s.empty());
}

void test_access() {
    int data[] = {10, 20, 30};
    Slice<int> s{data, 3};
    TEST_ASSERT_EQUAL(10, s[0]);
    TEST_ASSERT_EQUAL(20, s[1]);
    TEST_ASSERT_EQUAL(30, s[2]);
    s[1] = 99;
    TEST_ASSERT_EQUAL(99, data[1]);
}

void test_const_slice() {
    const int data[] = {7, 8, 9};
    Slice<const int> s{data, 3};
    TEST_ASSERT_EQUAL(7, s[0]);
    TEST_ASSERT_EQUAL(8, s[1]);
    TEST_ASSERT_EQUAL(9, s[2]);
}

void test_iterators() {
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

void test_sub() {
    int data[] = {1, 2, 3, 4, 5, 6};
    Slice<int> s{data, 6};
    auto sub = s.sub(2, 3);
    TEST_ASSERT_EQUAL_PTR(data + 2, sub.data());
    TEST_ASSERT_EQUAL(3, sub.size());
    TEST_ASSERT_EQUAL(3, sub[0]);
    TEST_ASSERT_EQUAL(4, sub[1]);
    TEST_ASSERT_EQUAL(5, sub[2]);
}

void test_first() {
    int data[] = {1, 2, 3, 4};
    Slice<int> s{data, 4};
    auto f = s.first(2);
    TEST_ASSERT_EQUAL_PTR(data, f.data());
    TEST_ASSERT_EQUAL(2, f.size());
    TEST_ASSERT_EQUAL(1, f[0]);
    TEST_ASSERT_EQUAL(2, f[1]);
}

void test_last() {
    int data[] = {1, 2, 3, 4};
    Slice<int> s{data, 4};
    auto l = s.last(2);
    TEST_ASSERT_EQUAL_PTR(data + 2, l.data());
    TEST_ASSERT_EQUAL(2, l.size());
    TEST_ASSERT_EQUAL(3, l[0]);
    TEST_ASSERT_EQUAL(4, l[1]);
}

void test_from_offset() {
    int data[] = {1, 2, 3, 4, 5};
    Slice<int> s{data, 5};
    auto off = s.fromOffset(2);
    TEST_ASSERT_EQUAL_PTR(data + 2, off.data());
    TEST_ASSERT_EQUAL(3, off.size());
    TEST_ASSERT_EQUAL(3, off[0]);
    TEST_ASSERT_EQUAL(4, off[1]);
    TEST_ASSERT_EQUAL(5, off[2]);
}

void test_empty() {
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

void test_copy_assign() {
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

void test_different_types() {
    char cdata[] = "abc";
    Slice<char> cs{cdata, 3};
    TEST_ASSERT_EQUAL('a', cs[0]);

    double ddata[] = {1.1, 2.2};
    Slice<double> ds{ddata, 2};
    TEST_ASSERT_EQUAL_FLOAT(1.1, ds[0]);
}

void test_array_constructor_int() {
    int data[] = {1, 2, 3, 4, 5};
    Slice<int> s{data};

    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(5, s.size());
    TEST_ASSERT_EQUAL(1, s[0]);
    TEST_ASSERT_EQUAL(3, s[2]);
    TEST_ASSERT_EQUAL(5, s[4]);
}

void test_array_constructor_char() {
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

void test_array_constructor_const() {
    const int data[] = {10, 20, 30};
    Slice<const int> s{data};

    TEST_ASSERT_EQUAL_PTR(data, s.data());
    TEST_ASSERT_EQUAL(3, s.size());
    TEST_ASSERT_EQUAL(10, s[0]);
    TEST_ASSERT_EQUAL(20, s[1]);
    TEST_ASSERT_EQUAL(30, s[2]);
}

void test_array_constructor_modification() {
    int data[] = {7, 8, 9};
    Slice<int> s{data};

    s[1] = 99;
    TEST_ASSERT_EQUAL(99, data[1]);
    TEST_ASSERT_EQUAL(99, s[1]);
}

void test_array_constructor_empty() {
    int data[1] = {42};
    Slice<int> s{data};

    TEST_ASSERT_EQUAL(1, s.size());
    TEST_ASSERT_EQUAL(42, s[0]);

    int *nullPtr = nullptr;
    Slice<int> s2{nullPtr, 0};
    TEST_ASSERT_NULL(s2.data());
    TEST_ASSERT_EQUAL(0, s2.size());
}

void test_array_constructor_foreach() {
    int data[] = {2, 4, 6, 8};
    Slice<int> s{data};

    int sum = 0;
    for (auto it = s.begin(); it != s.end(); it += 1) {
        sum += *it;
    }
    TEST_ASSERT_EQUAL(2 + 4 + 6 + 8, sum);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_default_ctor);
    RUN_TEST(test_ptr_size_ctor);
    RUN_TEST(test_array_constructor_int);
    RUN_TEST(test_array_constructor_char);
    RUN_TEST(test_array_constructor_const);
    RUN_TEST(test_array_constructor_modification);
    RUN_TEST(test_array_constructor_empty);
    RUN_TEST(test_array_constructor_foreach);
    RUN_TEST(test_access);
    RUN_TEST(test_const_slice);
    RUN_TEST(test_iterators);
    RUN_TEST(test_sub);
    RUN_TEST(test_first);
    RUN_TEST(test_last);
    RUN_TEST(test_from_offset);
    RUN_TEST(test_empty);
    RUN_TEST(test_copy_assign);
    RUN_TEST(test_different_types);
    return UNITY_END();
}