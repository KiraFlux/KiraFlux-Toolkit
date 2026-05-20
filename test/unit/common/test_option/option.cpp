#include <unity.h>

#include <kf/Option.hpp>
#include <type_traits>

using kf::Option;

struct Point {
    int x, y;

    static Point create() noexcept { return Point{10, 20}; }

    bool operator==(const Point &other) const { return x == other.x && y == other.y; }
};

template<typename T> struct OptionTester {
    inline static T value, default_value;

    static void some() noexcept {
        Option<T> opt{value};
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_FALSE(opt.isNone());
        TEST_ASSERT_TRUE(value == opt.value());
        TEST_ASSERT_TRUE(value == opt.valueOr(default_value));
    }

    static void none() noexcept {
        Option<T> opt{};
        TEST_ASSERT_TRUE(opt.isNone());
        TEST_ASSERT_FALSE(opt.isSome());
        TEST_ASSERT_TRUE(default_value == opt.valueOr(default_value));
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
        const Option<T> opt{value};
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(value == opt.value());
    }
};

#define RUN_OPTION_TESTS(__type__, __value__, __default_value__) \
    OptionTester<__type__>::value = __value__;                   \
    OptionTester<__type__>::default_value = __default_value__;   \
    RUN_TEST(OptionTester<__type__>::some);                      \
    RUN_TEST(OptionTester<__type__>::none);                      \
    RUN_TEST(OptionTester<__type__>::copy);                      \
    RUN_TEST(OptionTester<__type__>::copy_assignment);           \
    RUN_TEST(OptionTester<__type__>::move);                      \
    RUN_TEST(OptionTester<__type__>::move_assignment);           \
    RUN_TEST(OptionTester<__type__>::const_instance);

// Static assertions for triviality requirements
static_assert(std::is_trivially_copyable_v<Option<int>>);
static_assert(std::is_trivially_destructible_v<Option<int>>);
static_assert(std::is_trivially_copyable_v<Option<Point>>);
static_assert(std::is_trivially_destructible_v<Option<Point>>);

int main() {
    UNITY_BEGIN();

    RUN_OPTION_TESTS(int, 42, 0);
    RUN_OPTION_TESTS(float, 123.456f, 0);
    RUN_OPTION_TESTS(Point, Point::create(), Point{});

    return UNITY_END();
}