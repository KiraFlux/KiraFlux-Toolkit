#include <unity.h>

#include <kf/Result.hpp>

using kf::Result;

struct Error {
    char kind;

    bool operator==(const Error &other) const { return kind == other.kind; }
};

const int value{};
const Error error{0};

void test_result_ok() {
    kf::Result<int, Error> result{value};

    TEST_ASSERT_TRUE(result.isOk());
    TEST_ASSERT_TRUE(result.ok().hasValue());
    TEST_ASSERT_TRUE(result.ok().value() == value);

    TEST_ASSERT_FALSE(result.isError());
    TEST_ASSERT_FALSE(result.error().hasValue());
}

void test_result_is_error() {
    kf::Result<int, Error> result{error};

    TEST_ASSERT_TRUE(result.isError());
    TEST_ASSERT_TRUE(result.error().hasValue());
    TEST_ASSERT_TRUE(result.error().value() == error);

    TEST_ASSERT_FALSE(result.isOk());
    TEST_ASSERT_FALSE(result.ok().hasValue());
}

void test_void_result_is_ok() {
    kf::Result<void, Error> result{};

    TEST_ASSERT_TRUE(result.isOk());

    TEST_ASSERT_FALSE(result.isError());
    TEST_ASSERT_FALSE(result.error().hasValue());
}

void test_void_result_is_error() {
    kf::Result<void, Error> result{error};

    TEST_ASSERT_TRUE(result.isError());
    TEST_ASSERT_TRUE(result.error().hasValue());

    TEST_ASSERT_TRUE(result.error().value() == error);
    TEST_ASSERT_FALSE(result.isOk());
}

void test_result_copy() {
    kf::Result<int, Error> original{value};
    kf::Result<int, Error> copy{original};

    TEST_ASSERT_TRUE(original.isOk());
    TEST_ASSERT_TRUE(copy.isOk());

    TEST_ASSERT_EQUAL(copy.ok().value(), original.ok().value());
    TEST_ASSERT_NOT_EQUAL(&copy.ok().value(), &original.ok().value());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_result_ok);
    RUN_TEST(test_result_is_error);

    RUN_TEST(test_void_result_is_ok);
    RUN_TEST(test_void_result_is_error);

    RUN_TEST(test_result_copy);

    return UNITY_END();
}
