#include <runner.hpp>
#include <structures.hpp>

#include <kf/Option.hpp>

using kf::TrivialOption;

template<typename T> struct TestTrivialOption {
    static constexpr int value{12345}, default_value{0};

    static void some() noexcept {
        const TrivialOption<T> option = kf::someTrivial(T{value});

        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_FALSE(option.isNone());
    }

    static void none() noexcept {
        const TrivialOption<T> option = kf::none;

        TEST_ASSERT_TRUE(option.isNone());
        TEST_ASSERT_FALSE(option.isSome());
    }

    static void unwrap() noexcept {
        const TrivialOption<T> option = kf::someTrivial(T{value});

        TEST_ASSERT_TRUE(option.unwrap().value == value);
    }

    static void unwrap_or() noexcept {
        TEST_ASSERT_TRUE(TrivialOption<T>{kf::none}.unwrapOr(T{default_value}).value == default_value);
        TEST_ASSERT_TRUE(kf::someTrivial(T{value}).unwrapOr(T{default_value}).value == value);
    }

    static void reset() noexcept {
        TrivialOption<T> option = kf::someTrivial(T{value});
        option.reset();

        TEST_ASSERT_TRUE(option.isNone());
    }
};

#define RUN_TRIVIAL_OPTION_TESTS(__type__)            \
    RUN_TEST(TestTrivialOption<__type__>::some);      \
    RUN_TEST(TestTrivialOption<__type__>::none);      \
    RUN_TEST(TestTrivialOption<__type__>::unwrap);    \
    RUN_TEST(TestTrivialOption<__type__>::unwrap_or); \
    RUN_TEST(TestTrivialOption<__type__>::reset);

void runner() {
    using namespace kf::test;

    RUN_TRIVIAL_OPTION_TESTS(TrivialType);
}
