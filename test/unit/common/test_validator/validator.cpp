#include <kf/validation.hpp>

#include <unity.h>

using kf::Validator;

static constexpr auto logger = kf::Logger::create("Test");

struct TestConfig : kf::Validatable<TestConfig> {
    bool ok_is_true;

private:
    explicit constexpr TestConfig(bool a) noexcept : ok_is_true{a} {}

public:
    constexpr static TestConfig good() noexcept { return TestConfig{true}; }
    constexpr static TestConfig bad() noexcept { return TestConfig{false}; }

    void checkImpl(Validator &v) const noexcept {
        KF_VALIDATOR_CHECK(v, logger, ok_is_true);
    }
};

namespace test_validator {

void empty() {
    Validator v{};
    TEST_ASSERT_TRUE(v.passed());
}

void single_ok() {
    Validator v{};
    v.check(logger, true, "ok");
    TEST_ASSERT_TRUE(v.passed());
}

void single_fail() {
    Validator v{};
    v.check(logger, false, "fail");
    TEST_ASSERT_FALSE(v.passed());
}

void multiple() {
    Validator v{};
    v.check(logger, true, "ok1");
    v.check(logger, false, "fail1");
    v.check(logger, false, "fail2");
    TEST_ASSERT_FALSE(v.passed());
}

void macro() {
    Validator v{};
    KF_VALIDATOR_CHECK(v, logger, true);
    TEST_ASSERT_TRUE(v.passed());

    KF_VALIDATOR_CHECK(v, logger, false);
    TEST_ASSERT_FALSE(v.passed());
}

}// namespace test_validator

namespace test_validatable {

void good() {
    constexpr auto cfg{TestConfig::good()};
    Validator v{};
    cfg.check(v);
    TEST_ASSERT_TRUE(v.passed());
}

void bad() {
    constexpr auto cfg{TestConfig::bad()};
    Validator v{};
    cfg.check(v);
    TEST_ASSERT_FALSE(v.passed());
}

}// namespace test_validatable

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_validator::empty);
    RUN_TEST(test_validator::single_ok);
    RUN_TEST(test_validator::single_fail);
    RUN_TEST(test_validator::multiple);
    RUN_TEST(test_validator::macro);

    RUN_TEST(test_validatable::good);
    RUN_TEST(test_validatable::bad);

    return UNITY_END();
}