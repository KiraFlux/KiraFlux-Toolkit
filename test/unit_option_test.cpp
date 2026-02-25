#include <unity.h>

#include <kf/Option.hpp>

void test_option_has_value() {
    kf::Option<int> opt(42);
    TEST_ASSERT_TRUE(opt.hasValue());
    TEST_ASSERT_EQUAL(42, opt.value());
}

void test_option_empty() {
    kf::Option<int> opt;
    TEST_ASSERT_FALSE(opt.hasValue());
    TEST_ASSERT_EQUAL(0, opt.valueOr(0));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_option_has_value);
    RUN_TEST(test_option_empty);
    return UNITY_END();
}

