#include <unity.h>

#include <kf/Option.hpp>
#include <type_traits>

using kf::Option;

struct Point {
    int x, y;
    bool operator==(const Point &other) const { return x == other.x && y == other.y; }
};

template<typename T> void test_option_with_values(const T &value, const T &defaultVal) {
    // Конструктор со значением
    {
        Option<T> opt{value};
        TEST_ASSERT_TRUE(opt.hasValue());
        TEST_ASSERT_TRUE(value == opt.value());
        TEST_ASSERT_TRUE(value == opt.valueOr(defaultVal));
    }

    // Конструктор по умолчанию (пустой)
    {
        Option<T> opt{};
        TEST_ASSERT_FALSE(opt.hasValue());
        TEST_ASSERT_TRUE(defaultVal == opt.valueOr(defaultVal));
    }

    // Копирование
    {
        Option<T> original{value};
        Option<T> copy{original};
        TEST_ASSERT_TRUE(original.hasValue());
        TEST_ASSERT_TRUE(copy.hasValue());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
        TEST_ASSERT_FALSE(&original.value() == &copy.value());
    }

    // Присваивание копированием
    {
        Option<T> original{value};
        Option<T> copy;
        copy = original;
        TEST_ASSERT_TRUE(original.hasValue());
        TEST_ASSERT_TRUE(copy.hasValue());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == copy.value());
    }

    // Перемещение (для тривиальных типов эквивалентно копированию)
    {
        Option<T> original{value};
        Option<T> moved{std::move(original)};
        TEST_ASSERT_TRUE(original.hasValue());
        TEST_ASSERT_TRUE(moved.hasValue());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    // Присваивание перемещением
    {
        Option<T> original{value};
        Option<T> moved{};
        moved = std::move(original);
        TEST_ASSERT_TRUE(original.hasValue());
        TEST_ASSERT_TRUE(moved.hasValue());
        TEST_ASSERT_TRUE(value == original.value());
        TEST_ASSERT_TRUE(value == moved.value());
    }

    // Константный объект (проверяет наличие const-версии value())
    {
        const Option<T> opt{value};
        TEST_ASSERT_TRUE(opt.hasValue());
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

// Статические проверки требований к тривиальности
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