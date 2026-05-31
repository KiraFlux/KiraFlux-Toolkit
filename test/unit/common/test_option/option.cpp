#include <unity.h>

#include <type_traits>

#include <kf/Option.hpp>

#include "structures.hpp"

using kf::Option;

using kf::test::Point;

template<typename T> struct TestOption {
    inline static T value{};
    static constexpr T default_value{};

    static constexpr auto map_result{12345};
    static constexpr auto mapper{[](const T &) { return map_result; }};

    static void some() noexcept {
        auto option_some = kf::some(value);
        TEST_ASSERT_TRUE(option_some.isSome());
        TEST_ASSERT_FALSE(option_some.isNone());
        TEST_ASSERT_TRUE(value == option_some.unwrap());
        TEST_ASSERT_TRUE(value == option_some.unwrapOr(default_value));
    }

    static void none() noexcept {
        Option<T> option_none = kf::none;
        TEST_ASSERT_TRUE(option_none.isNone());
        TEST_ASSERT_FALSE(option_none.isSome());
        TEST_ASSERT_TRUE(default_value == option_none.unwrapOr(default_value));
    }

    static void copy() noexcept {
        auto original = kf::some(value);
        auto copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.unwrap());
        TEST_ASSERT_TRUE(value == copy.unwrap());
        TEST_ASSERT_FALSE(&original.unwrap() == &copy.unwrap());
    }

    static void copy_assignment() noexcept {
        auto original = kf::some(value);
        Option<T> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.unwrap());
        TEST_ASSERT_TRUE(value == copy.unwrap());
    }

    static void move() noexcept {
        auto original = kf::some(value);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(original.isNone());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == moved.unwrap());
    }

    static void move_assignment() noexcept {
        auto original = kf::some(value);
        Option<T> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(original.isNone());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == moved.unwrap());
    }

    static void const_instance() noexcept {
        const auto option = kf::some(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.unwrap());
    }

    static void value_get() noexcept {
        auto option = kf::some(value);
        TEST_ASSERT_TRUE(value == option.unwrap());
    }

    static void value_get_const() noexcept {
        const auto option = kf::some(value);
        TEST_ASSERT_TRUE(value == option.unwrap());
    }

    static void reassign() noexcept {
        auto option = kf::some(default_value);
        option = kf::some(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.unwrap());

        option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void reset() noexcept {
        auto option = kf::some(value);
        option.reset();
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void map_some() noexcept {
        auto mapped_some = kf::some(value).map(mapper);
        TEST_ASSERT_TRUE(mapped_some.isSome());
        TEST_ASSERT_TRUE(mapped_some.unwrap() == map_result);
    }

    static void map_none() noexcept {
        auto mapped_none = Option<T>(kf::none).map(mapper);
        TEST_ASSERT_TRUE(mapped_none.isNone());
    }
};

#define RUN_OPTION_TESTS(__type__, __value__)        \
    TestOption<__type__>::value = __value__;         \
    RUN_TEST(TestOption<__type__>::some);            \
    RUN_TEST(TestOption<__type__>::none);            \
    RUN_TEST(TestOption<__type__>::copy);            \
    RUN_TEST(TestOption<__type__>::copy_assignment); \
    RUN_TEST(TestOption<__type__>::move);            \
    RUN_TEST(TestOption<__type__>::move_assignment); \
    RUN_TEST(TestOption<__type__>::const_instance);  \
    RUN_TEST(TestOption<__type__>::value_get);       \
    RUN_TEST(TestOption<__type__>::value_get_const); \
    RUN_TEST(TestOption<__type__>::reassign);        \
    RUN_TEST(TestOption<__type__>::reset);           \
    RUN_TEST(TestOption<__type__>::map_some);        \
    RUN_TEST(TestOption<__type__>::map_none)

int main() {
    UNITY_BEGIN();

    RUN_OPTION_TESTS(int, 42);
    RUN_OPTION_TESTS(float, 123.456f);
    RUN_OPTION_TESTS(Point, Point::create());

    return UNITY_END();
}