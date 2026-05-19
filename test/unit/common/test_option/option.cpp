#include <unity.h>

#include <kf/Option.hpp>
#include <type_traits>

using kf::Option;

struct Point {
    int x, y;
    bool operator==(const Point &other) const { return x == other.x && y == other.y; }
};

template<typename T> void test_option_with_values(const T &value, const T &default_value) {
    // Constructor with a value
    {
        Option<T> opt{value};
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_FALSE(opt.isNone());
        TEST_ASSERT_TRUE(value == opt.value());
        TEST_ASSERT_TRUE(value == opt.valueOr(default_value));
    }

    // Default constructor (empty)
    {
        Option<T> opt{};
        TEST_ASSERT_TRUE(opt.isNone());
        TEST_ASSERT_FALSE(opt.isSome());
        TEST_ASSERT_TRUE(default_value == opt.valueOr(default_value));
    }

    // Copy construction
    {
        Option<T> original{value};
        Option<T> copy{original};
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
        TEST_ASSERT_FALSE(&original.value() == &copy.value());
    }

    // Copy assignment
    {
        Option<T> original{value};
        Option<T> copy;
        copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
    }

    // Move construction (for trivial types equivalent to copy)
    {
        Option<T> original{value};
        Option<T> moved{std::move(original)};
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    // Move assignment
    {
        Option<T> original{value};
        Option<T> moved{};
        moved = std::move(original);
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    // Const object (verifies const version of value())
    {
        const Option<T> opt{value};
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(value == opt.value());
    }
}

void test_option_int() {
    test_option_with_values<int>(42, 0);
}

void test_option_float() {
    test_option_with_values<float>(3.14f, 0.0f);
}

void test_option_point() {
    Point p{10, 20};
    Point default_p{0, 0};
    test_option_with_values<Point>(p, default_p);
}

// Static assertions for triviality requirements
static_assert(std::is_trivially_copyable_v<Option<int>>);
static_assert(std::is_trivially_destructible_v<Option<int>>);
static_assert(std::is_trivially_copyable_v<Option<Point>>);
static_assert(std::is_trivially_destructible_v<Option<Point>>);

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_option_int);
    RUN_TEST(test_option_float);
    RUN_TEST(test_option_point);
    return UNITY_END();
}