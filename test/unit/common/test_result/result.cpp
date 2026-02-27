#include <unity.h>

#include <kf/Result.hpp>

using kf::Result;

struct Error {
    char kind;
    bool operator==(const Error &other) const { return kind == other.kind; }
};

const Error error{'A'};

template<typename T> struct ResultTester {
    static constexpr T value{};

    static void test_is_ok() {
        Result<T, Error> r{value};
        TEST_ASSERT_TRUE(r.isOk());
        TEST_ASSERT_TRUE(r.ok().hasValue());
        TEST_ASSERT_TRUE(value == r.ok().value());
        TEST_ASSERT_FALSE(r.error().hasValue());
    }

    static void test_is_error() {
        Result<T, Error> r{error};
        TEST_ASSERT_TRUE(r.isError());
        TEST_ASSERT_FALSE(r.ok().hasValue());
        TEST_ASSERT_TRUE(r.error().hasValue());
        TEST_ASSERT_TRUE(error == r.error().value());
    }

    static void test_copy() {
        Result<T, Error> a{value};
        Result<T, Error> b{a};
        TEST_ASSERT_TRUE(a.isOk());
        TEST_ASSERT_TRUE(b.isOk());
        TEST_ASSERT_TRUE(a.ok().value() == b.ok().value());
        TEST_ASSERT_NOT_EQUAL(&a.ok().value(), &b.ok().value());
    }

    static void test_const() {
        const Result<T, Error> r{value};
        TEST_ASSERT_TRUE(r.isOk());
        const auto ok_opt = r.ok();
        TEST_ASSERT_TRUE(value == ok_opt.value());
    }
};

void test_void_ok() {
    Result<void, Error> r{};
    TEST_ASSERT_TRUE(r.isOk());
    TEST_ASSERT_FALSE(r.isError());
    TEST_ASSERT_FALSE(r.error().hasValue());
}

void test_void_error() {
    Result<void, Error> r{error};
    TEST_ASSERT_TRUE(r.isError());
    TEST_ASSERT_FALSE(r.isOk());
    TEST_ASSERT_TRUE(r.error().hasValue());
    TEST_ASSERT_TRUE(error == r.error().value());
}

void test_void_copy() {
    Result<void, Error> a{};
    Result<void, Error> b{a};
    TEST_ASSERT_TRUE(a.isOk());
    TEST_ASSERT_TRUE(b.isOk());
}

void test_void_move() {
    Result<void, Error> a{};
    Result<void, Error> b{std::move(a)};
    TEST_ASSERT_TRUE(a.isOk());
    TEST_ASSERT_TRUE(b.isOk());
}

void test_void_const() {
    const Result<void, Error> r{};
    TEST_ASSERT_TRUE(r.isOk());
}

#define RUN_RESULT_TESTS(T)                   \
    RUN_TEST(ResultTester<T>::test_is_ok);    \
    RUN_TEST(ResultTester<T>::test_is_error); \
    RUN_TEST(ResultTester<T>::test_copy);     \
    RUN_TEST(ResultTester<T>::test_const)

int main() {
    UNITY_BEGIN();

    RUN_RESULT_TESTS(int);
    RUN_RESULT_TESTS(float);

    RUN_TEST(test_void_ok);
    RUN_TEST(test_void_error);
    RUN_TEST(test_void_copy);
    RUN_TEST(test_void_move);
    RUN_TEST(test_void_const);

    return UNITY_END();
}