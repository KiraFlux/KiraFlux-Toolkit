#include <unity.h>

#include <kf/Option.hpp>
#include <type_traits>

using kf::Option;

struct Point {
    int x, y;

    static Point create() noexcept { return Point{10, 20}; }

    bool operator==(const Point &other) const { return x == other.x and y == other.y; }
};

template<typename T> struct OptionTester {

    // Static assertions for triviality requirements
    static_assert(std::is_trivially_copyable_v<Option<T>>);
    static_assert(std::is_trivially_destructible_v<Option<T>>);

    inline static T value{};
    static constexpr T default_value{};

    static constexpr auto map_result{12345};
    static constexpr auto mapper{[](const T &) { return map_result; }};

    static void some() noexcept {
        auto option_some = kf::some(value);
        TEST_ASSERT_TRUE(option_some.isSome());
        TEST_ASSERT_FALSE(option_some.isNone());
        TEST_ASSERT_TRUE(value == option_some.value());
        TEST_ASSERT_TRUE(value == option_some.valueOr(default_value));
    }

    static void none() noexcept {
        Option<T> option_none = kf::none;
        TEST_ASSERT_TRUE(option_none.isNone());
        TEST_ASSERT_FALSE(option_none.isSome());
        TEST_ASSERT_TRUE(default_value == option_none.valueOr(default_value));
    }

    static void copy() noexcept {
        auto original = kf::some(value);
        auto copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
        TEST_ASSERT_FALSE(&original.value() == &copy.value());
    }

    static void copy_assignment() noexcept {
        auto original = kf::some(value);
        Option<T> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
    }

    static void move() noexcept {
        auto original = kf::some(value);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(original.isSome());// moved-from is still some (trivial)
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    static void move_assignment() noexcept {
        auto original = kf::some(value);
        Option<T> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    static void const_instance() noexcept {
        const auto option = kf::some(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void value_get() noexcept {
        auto option = kf::some(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void value_get_const() noexcept {
        const auto option = kf::some(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void reassign() noexcept {
        auto option = kf::some(default_value);
        option = kf::some(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());

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
        TEST_ASSERT_TRUE(mapped_some.value() == map_result);
    }

    static void map_none() noexcept {
        auto mapped_none = Option<T>(kf::none).map(mapper);
        TEST_ASSERT_TRUE(mapped_none.isNone());
    }
};

#define RUN_OPTION_TESTS(__type__, __value__)          \
    OptionTester<__type__>::value = __value__;         \
    RUN_TEST(OptionTester<__type__>::some);            \
    RUN_TEST(OptionTester<__type__>::none);            \
    RUN_TEST(OptionTester<__type__>::copy);            \
    RUN_TEST(OptionTester<__type__>::copy_assignment); \
    RUN_TEST(OptionTester<__type__>::move);            \
    RUN_TEST(OptionTester<__type__>::move_assignment); \
    RUN_TEST(OptionTester<__type__>::const_instance);  \
    RUN_TEST(OptionTester<__type__>::value_get);       \
    RUN_TEST(OptionTester<__type__>::value_get_const); \
    RUN_TEST(OptionTester<__type__>::reassign);        \
    RUN_TEST(OptionTester<__type__>::reset);           \
    RUN_TEST(OptionTester<__type__>::map_some);        \
    RUN_TEST(OptionTester<__type__>::map_none);

int main() {
    UNITY_BEGIN();

    RUN_OPTION_TESTS(int, 42);
    RUN_OPTION_TESTS(float, 123.456f);
    RUN_OPTION_TESTS(Point, Point::create());

    return UNITY_END();
}