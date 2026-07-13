#include <runner.hpp>

#include <kf/Option.hpp>

using kf::TrivialOption;

template<typename T> struct TestOptionalRealNumber {
    static_assert(sizeof(TrivialOption<T>) == sizeof(T));

    static constexpr T value{static_cast<T>(1)}, default_value{static_cast<T>(0)};

    static void some() noexcept {
        const TrivialOption<T> option = kf::someTrivial(value);

        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_FALSE(option.isNone());
    }

    static void none() noexcept {
        const TrivialOption<T> option = kf::none;

        TEST_ASSERT_FALSE(option.isSome());
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void unwrap() noexcept {
        const TrivialOption<T> option = kf::someTrivial(value);

        TEST_ASSERT_TRUE(option.unwrap() == value);
    }

    static void unwrap_or() noexcept {
        const TrivialOption<T> option_none = kf::none;
        const TrivialOption<T> option_some = kf::someTrivial(value);

        TEST_ASSERT_TRUE(option_some.unwrapOr(default_value) == value);
        TEST_ASSERT_TRUE(option_none.unwrapOr(default_value) == default_value);
    }

    static void reset() noexcept {
        TrivialOption<T> option = kf::someTrivial(value);
        option.reset();

        TEST_ASSERT_TRUE(option.isNone());
    }
};

#define RUN_OPTIONAL_REAL_NUMBER_TESTS(__type__)           \
    RUN_TEST(TestOptionalRealNumber<__type__>::some);      \
    RUN_TEST(TestOptionalRealNumber<__type__>::none);      \
    RUN_TEST(TestOptionalRealNumber<__type__>::unwrap);    \
    RUN_TEST(TestOptionalRealNumber<__type__>::unwrap_or); \
    RUN_TEST(TestOptionalRealNumber<__type__>::reset);

void run_tests() {
    using long_double = long double;

    RUN_OPTIONAL_REAL_NUMBER_TESTS(float);
    RUN_OPTIONAL_REAL_NUMBER_TESTS(double);
    RUN_OPTIONAL_REAL_NUMBER_TESTS(long_double);
    RUN_OPTIONAL_REAL_NUMBER_TESTS(kf::usize);
}