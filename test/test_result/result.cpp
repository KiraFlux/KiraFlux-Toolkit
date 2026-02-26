#include <unity.h>

#include <kf/Result.hpp>

using kf::Result;

struct Error {
    bool operator==(const Error &other) { return true; }
};

const int value{};
const Error error{};

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

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_result_ok);
    RUN_TEST(test_result_is_error);
    return UNITY_END();
}
