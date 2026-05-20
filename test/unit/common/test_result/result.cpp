#include <kf/Result.hpp>

#include <unity.h>

using kf::Result;

struct Error {
    char kind;
    bool operator==(const Error &other) const { return kind == other.kind; }
};

const Error error{'A'};

template<typename T> struct ResultTester {
    static constexpr T value{};

    static void is_ok() {
        Result<T, Error> r{value};
        TEST_ASSERT_TRUE(r.isOk());
        TEST_ASSERT_TRUE(value == r.ok());
    }

    static void is_error() {
        Result<T, Error> r{error};
        TEST_ASSERT_TRUE(r.isError());
        TEST_ASSERT_TRUE(error == r.error());
    }

    static void copy() {
        Result<T, Error> a{value};
        Result<T, Error> b{a};
        TEST_ASSERT_TRUE(a.isOk());
        TEST_ASSERT_TRUE(b.isOk());
        TEST_ASSERT_TRUE(a.ok() == b.ok());
        TEST_ASSERT_NOT_EQUAL(&a.ok(), &b.ok());
    }

    static void copy_assignment() {
        Result<T, Error> a{value};
        Result<T, Error> b{a};
        b = a;
        TEST_ASSERT_TRUE(a.isOk());
        TEST_ASSERT_TRUE(b.isOk());
        TEST_ASSERT_TRUE(a.ok() == b.ok());
        TEST_ASSERT_NOT_EQUAL(&a.ok(), &b.ok());
    }

    static void move() {
        Result<T, Error> a{value};
        Result<T, Error> b{std::move(a)};
        TEST_ASSERT_TRUE(a.isOk());
        TEST_ASSERT_TRUE(b.isOk());
        TEST_ASSERT_TRUE(a.ok() == b.ok());
        TEST_ASSERT_NOT_EQUAL(&a.ok(), &b.ok());
    }

    static void move_assignment() {
        Result<T, Error> a{value};
        Result<T, Error> b{std::move(a)};
        b = std::move(a);
        TEST_ASSERT_TRUE(a.isOk());
        TEST_ASSERT_TRUE(b.isOk());
        TEST_ASSERT_TRUE(a.ok() == b.ok());
        TEST_ASSERT_NOT_EQUAL(&a.ok(), &b.ok());
    }

    static void const_instance() {
        const Result<T, Error> r{value};
        TEST_ASSERT_TRUE(r.isOk());
        TEST_ASSERT_TRUE(value == r.ok());
    }

    static void value_get() {
        Result<T, Error> r{value};
        TEST_ASSERT_TRUE(value == r.ok());
    }

    static void value_get_const() {
        const Result<T, Error> r{value};
        TEST_ASSERT_TRUE(value == r.ok());
    }

    static void value_set_copy() {
        Result<T, Error> r{T{}};
        r.ok(value);
        TEST_ASSERT_TRUE(value == r.ok());
    }

    static void value_set_move() {
        Result<T, Error> r{T{}};
        r.ok(std::move(value));
        TEST_ASSERT_TRUE(value == r.ok());
    }

    static constexpr auto map_result {"test"};

    static void ok_map_ok() {
        const auto mapped = Result<T, Error>{value}.map([](T) { return map_result; });

        TEST_ASSERT_TRUE(mapped.isOk());
        TEST_ASSERT_TRUE(map_result == mapped.ok());
    }

    static void error_map_ok() {
        const auto mapped = Result<T, Error>{error}.map([](T) { return map_result; });

        TEST_ASSERT_TRUE(mapped.isError());
        TEST_ASSERT_TRUE(error == mapped.error());
    }

    static void ok_map_error() {
        const auto mapped = Result<T, Error>{value}.mapError([](Error) { return map_result; });

        TEST_ASSERT_TRUE(mapped.isOk());
        TEST_ASSERT_TRUE(value == mapped.ok());
    }

    static void error_map_error() {
        const auto mapped = Result<T, Error>{error}.mapError([](Error) { return map_result; });

        TEST_ASSERT_TRUE(mapped.isError());
        TEST_ASSERT_TRUE(map_result == mapped.error());
    }
};

void test_void_ok() {
    Result<void, Error> r{};
    TEST_ASSERT_TRUE(r.isOk());
    TEST_ASSERT_FALSE(r.isError());
}

void test_void_error() {
    Result<void, Error> r{error};
    TEST_ASSERT_TRUE(r.isError());
    TEST_ASSERT_FALSE(r.isOk());
    TEST_ASSERT_TRUE(error == r.error());
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

#define RUN_RESULT_TESTS(T)                     \
    RUN_TEST(ResultTester<T>::is_ok);           \
    RUN_TEST(ResultTester<T>::is_error);        \
    RUN_TEST(ResultTester<T>::copy);            \
    RUN_TEST(ResultTester<T>::copy_assignment); \
    RUN_TEST(ResultTester<T>::move);            \
    RUN_TEST(ResultTester<T>::move_assignment); \
    RUN_TEST(ResultTester<T>::const_instance);  \
    RUN_TEST(ResultTester<T>::value_get);       \
    RUN_TEST(ResultTester<T>::value_get_const); \
    RUN_TEST(ResultTester<T>::value_set_copy);  \
    RUN_TEST(ResultTester<T>::value_set_move);  \
    RUN_TEST(ResultTester<T>::ok_map_ok);       \
    RUN_TEST(ResultTester<T>::error_map_ok);    \
    RUN_TEST(ResultTester<T>::ok_map_error);    \
    RUN_TEST(ResultTester<T>::error_map_error);

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