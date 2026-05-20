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
        Option<T> option_some{value};
        TEST_ASSERT_TRUE(option_some.isSome());
        TEST_ASSERT_FALSE(option_some.isNone());
        TEST_ASSERT_TRUE(value == option_some.value());
        TEST_ASSERT_TRUE(value == option_some.valueOr(default_value));
    }

    static void none() noexcept {
        Option<T> option_none{};
        TEST_ASSERT_TRUE(option_none.isNone());
        TEST_ASSERT_FALSE(option_none.isSome());
        TEST_ASSERT_TRUE(default_value == option_none.valueOr(default_value));
    }

    static void copy() noexcept {
        Option<T> original{value};
        Option<T> copy{original};
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
        TEST_ASSERT_FALSE(&original.value() == &copy.value());
    }

    static void copy_assignment() noexcept {
        Option<T> original{value};
        Option<T> copy;
        copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
    }

    static void move() noexcept {
        Option<T> original{value};
        Option<T> moved{std::move(original)};
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    static void move_assignment() noexcept {
        Option<T> original{value};
        Option<T> moved{};
        moved = std::move(original);
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    static void const_instance() noexcept {
        const Option<T> option{value};
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void value_get() noexcept {
        Option<T> option{value};
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void value_get_const() noexcept {
        const Option<T> option{value};
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void value_set_copy() noexcept {
        Option<T> option{default_value};
        option.value(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void value_set_move() noexcept {
        Option<T> option{default_value};
        option.value(std::move(value));
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void reset() noexcept {
        Option<T> option{value};
        option.reset();
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void map_some() noexcept {
        const auto mapped_some = Option<T>{value}.map(mapper);
        TEST_ASSERT_TRUE(mapped_some.isSome());
        TEST_ASSERT_TRUE(mapped_some.value() == map_result);
    }

    static void map_none() noexcept {
        const auto mapped_none = Option<T>{}.map(mapper);
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
    RUN_TEST(OptionTester<__type__>::value_set_copy);  \
    RUN_TEST(OptionTester<__type__>::value_set_move);  \
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