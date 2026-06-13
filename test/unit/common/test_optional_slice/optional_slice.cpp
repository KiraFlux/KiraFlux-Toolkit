#include <runner.hpp>

#include <kf/Option.hpp>
#include <kf/Slice.hpp>

using kf::Option;
using kf::Slice;

template<typename T> struct TestOptionalSlice {
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
        TEST_ASSERT_EQUAL_PTR(data_a, option.unwrap().data());
        TEST_ASSERT_EQUAL(data_a_size, option.unwrap().size());
    }

    static void none() {
        Option<Slice<T>> option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
        TEST_ASSERT_FALSE(option.isSome());
        const auto value = option.unwrapOr(slice_b);
        TEST_ASSERT_EQUAL_PTR(data_b, value.data());
        TEST_ASSERT_EQUAL(data_b_size, value.size());
    }

    static void empty_slice() {
        T dummy{};
        Slice<T> empty_slice{&dummy, 0};
        auto option = kf::some(empty_slice);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_EQUAL_PTR(&dummy, option.unwrap().data());
        TEST_ASSERT_EQUAL(0u, option.unwrap().size());
    }

    static void copy() {
        const auto original = kf::some(slice_a);
        const auto copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, copy.unwrap().data());
    }

    static void copy_assignment() {
        auto original = kf::some(slice_a);
        Option<Slice<T>> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, copy.unwrap().data());
    }

    static void move() {
        auto original = kf::some(slice_a);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, moved.unwrap().data());
    }

    static void move_assignment() {
        auto original = kf::some(slice_a);
        Option<Slice<T>> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, moved.unwrap().data());
    }

    static void const_instance() {
        const auto option = kf::some(slice_a);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_EQUAL_PTR(data_a, option.unwrap().data());
    }

    static void value_get() {
        auto option = kf::some(slice_a);
        TEST_ASSERT_EQUAL_PTR(data_a, option.unwrap().data());
        TEST_ASSERT_EQUAL(data_a_size, option.unwrap().size());
    }

    static void reassign() {
        auto option = kf::some(slice_a);
        option = kf::some(slice_b);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_EQUAL_PTR(data_b, option.unwrap().data());

        option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void reset() {
        auto option = kf::some(slice_a);
        option.reset();
        TEST_ASSERT_TRUE(option.isNone());
    }
};

#define RUN_OPTIONAL_SLICE_TESTS(__type__)                  \
    RUN_TEST(TestOptionalSlice<__type__>::some);            \
    RUN_TEST(TestOptionalSlice<__type__>::none);            \
    RUN_TEST(TestOptionalSlice<__type__>::empty_slice);     \
    RUN_TEST(TestOptionalSlice<__type__>::copy);            \
    RUN_TEST(TestOptionalSlice<__type__>::copy_assignment); \
    RUN_TEST(TestOptionalSlice<__type__>::move);            \
    RUN_TEST(TestOptionalSlice<__type__>::move_assignment); \
    RUN_TEST(TestOptionalSlice<__type__>::const_instance);  \
    RUN_TEST(TestOptionalSlice<__type__>::value_get);       \
    RUN_TEST(TestOptionalSlice<__type__>::reassign);        \
    RUN_TEST(TestOptionalSlice<__type__>::reset)

void run_tests() {
    RUN_OPTIONAL_SLICE_TESTS(int);
}