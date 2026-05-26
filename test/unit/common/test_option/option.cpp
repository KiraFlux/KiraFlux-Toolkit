#include <unity.h>

#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <type_traits>

using kf::Option;
using kf::Slice;

struct Point {
    int x, y;
    static Point create() noexcept { return Point{10, 20}; }
    bool operator==(const Point &other) const { return x == other.x and y == other.y; }
};

template<typename T> struct TestOption {
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
        TEST_ASSERT_TRUE(original.isSome());
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
        TEST_ASSERT_TRUE(value == option.value());
    }

    static void value_get_const() noexcept {
        const auto option = kf::some(value);
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

template<typename T> struct TestReferenceOption {
    static_assert(sizeof(Option<T &>) == sizeof(T *));

    inline static T value{};
    inline static T default_value{};

    static void some() {
        auto option = kf::someRef(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_FALSE(option.isNone());
        TEST_ASSERT_TRUE(&value == &option.value());
        T &def = default_value;
        TEST_ASSERT_TRUE(&value == &option.valueOr(def));
    }

    static void none() {
        Option<T &> option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
        TEST_ASSERT_FALSE(option.isSome());
        T &def = default_value;
        TEST_ASSERT_TRUE(&def == &option.valueOr(def));
    }

    static void copy() {
        auto original = kf::someRef(value);
        auto copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&value == &original.value());
        TEST_ASSERT_TRUE(&value == &copy.value());
        TEST_ASSERT_TRUE(&original.value() == &copy.value());
    }

    static void copy_assignment() {
        auto original = kf::someRef(value);
        Option<T &> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&value == &original.value());
        TEST_ASSERT_TRUE(&value == &copy.value());
    }

    static void move() {
        auto original = kf::someRef(value);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&value == &moved.value());
    }

    static void move_assignment() {
        auto original = kf::someRef(value);
        Option<T &> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&value == &moved.value());
    }

    static void const_instance() {
        const auto option = kf::someRef(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(&value == &option.value());
    }

    static void value_get() {
        auto option = kf::someRef(value);
        TEST_ASSERT_TRUE(&value == &option.value());
    }

    static void reassign() {
        T other = default_value;
        auto option = kf::someRef(value);
        option = kf::someRef(other);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(&other == &option.value());

        option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void reset() {
        auto option = kf::someRef(value);
        option.reset();
        TEST_ASSERT_TRUE(option.isNone());
    }
};

#define RUN_REFERENCE_OPTION_TESTS(__type__, __value__)       \
    TestReferenceOption<__type__>::value = __value__;         \
    RUN_TEST(TestReferenceOption<__type__>::some);            \
    RUN_TEST(TestReferenceOption<__type__>::none);            \
    RUN_TEST(TestReferenceOption<__type__>::copy);            \
    RUN_TEST(TestReferenceOption<__type__>::copy_assignment); \
    RUN_TEST(TestReferenceOption<__type__>::move);            \
    RUN_TEST(TestReferenceOption<__type__>::move_assignment); \
    RUN_TEST(TestReferenceOption<__type__>::const_instance);  \
    RUN_TEST(TestReferenceOption<__type__>::value_get);       \
    RUN_TEST(TestReferenceOption<__type__>::reassign);        \
    RUN_TEST(TestReferenceOption<__type__>::reset)

template<typename T> struct TestSliceOption {
    static_assert(sizeof(Option<Slice<T>>) == (sizeof(T *) * 2));

    static constexpr auto data_a_size{10u}, data_b_size{20u};
    inline static T data_a[data_a_size]{};
    inline static T data_b[data_b_size]{};

    inline static Slice<T> slice_a{data_a};
    inline static Slice<T> slice_b{data_b};

    static void some() {
        auto option = kf::some(slice_a);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_FALSE(option.isNone());
        TEST_ASSERT_EQUAL_PTR(data_a, option.value().data());
        TEST_ASSERT_EQUAL(data_a_size, option.value().size());
    }

    static void none() {
        Option<Slice<T>> option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
        TEST_ASSERT_FALSE(option.isSome());
        const auto value = option.valueOr(slice_b);
        TEST_ASSERT_EQUAL_PTR(data_b, value.data());
        TEST_ASSERT_EQUAL(data_b_size, value.size());
    }

    static void empty_slice() {
        T dummy{};
        Slice<T> empty_slice{&dummy, 0};
        auto option = kf::some(empty_slice);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_EQUAL_PTR(&dummy, option.value().data());
        TEST_ASSERT_EQUAL(0u, option.value().size());
    }

    static void copy() {
        const auto original = kf::some(slice_a);
        const auto copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, copy.value().data());
    }

    static void copy_assignment() {
        auto original = kf::some(slice_a);
        Option<Slice<T>> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, copy.value().data());
    }

    static void move() {
        auto original = kf::some(slice_a);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, moved.value().data());
    }

    static void move_assignment() {
        auto original = kf::some(slice_a);
        Option<Slice<T>> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, moved.value().data());
    }

    static void const_instance() {
        const auto option = kf::some(slice_a);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, option.value().data());
    }

    static void value_get() {
        auto option = kf::some(slice_a);
        TEST_ASSERT_EQUAL_PTR(data_a, option.value().data());
        TEST_ASSERT_EQUAL(data_a_size, option.value().size());
    }

    static void reassign() {
        auto option = kf::some(slice_a);
        option = kf::some(slice_b);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_EQUAL_PTR(data_b, option.value().data());

        option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void reset() {
        auto option = kf::some(slice_a);
        option.reset();
        TEST_ASSERT_TRUE(option.isNone());
    }
};

#define RUN_SLICE_OPTION_TESTS(T)                  \
    RUN_TEST(TestSliceOption<T>::some);            \
    RUN_TEST(TestSliceOption<T>::none);            \
    RUN_TEST(TestSliceOption<T>::empty_slice);     \
    RUN_TEST(TestSliceOption<T>::copy);            \
    RUN_TEST(TestSliceOption<T>::copy_assignment); \
    RUN_TEST(TestSliceOption<T>::move);            \
    RUN_TEST(TestSliceOption<T>::move_assignment); \
    RUN_TEST(TestSliceOption<T>::const_instance);  \
    RUN_TEST(TestSliceOption<T>::value_get);       \
    RUN_TEST(TestSliceOption<T>::reassign);        \
    RUN_TEST(TestSliceOption<T>::reset)

int main() {
    UNITY_BEGIN();

    RUN_OPTION_TESTS(int, 42);
    RUN_OPTION_TESTS(float, 123.456f);
    RUN_OPTION_TESTS(Point, Point::create());

    RUN_REFERENCE_OPTION_TESTS(Point, Point::create());

    RUN_SLICE_OPTION_TESTS(int);

    return UNITY_END();
}