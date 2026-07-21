#include <runner.hpp>

#include <type_traits>

#include <kf/Option.hpp>

#include "structures.hpp"

#define CHECK_COPY_CONSTRUCTOR_CALLS(__type__, __expected__) \
    if constexpr (__type__::tracked_copy) { TEST_ASSERT_EQUAL(__expected__, __type__::copy_constructor_calls); }

#define CHECK_COPY_ASSIGNMENT_CALLS(__type__, __expected__) \
    if constexpr (__type__::tracked_copy) { TEST_ASSERT_EQUAL(__expected__, __type__::copy_assignment_calls); }

#define CHECK_MOVE_CONSTRUCTOR_CALLS(__type__, __expected__) \
    if constexpr (__type__::tracked_move) { TEST_ASSERT_EQUAL(__expected__, __type__::move_constructor_calls); }

#define CHECK_MOVE_ASSIGNMENT_CALLS(__type__, __expected__) \
    if constexpr (__type__::tracked_move) { TEST_ASSERT_EQUAL(__expected__, __type__::move_assignment_calls); }

#define CHECK_DESTRUCTOR_CALLS(__type__, __expected__) \
    if constexpr (__type__::tracked_constructor_destructor) { TEST_ASSERT_EQUAL(__expected__, __type__::destructor_calls); }

#define CHECK_CONSTRUCTOR_CALLS(__type__, __expected__) \
    if constexpr (__type__::tracked_constructor_destructor) { TEST_ASSERT_EQUAL(__expected__, __type__::constructor_calls); }

using kf::Option;

template<typename T> struct TestOption {
    static constexpr int value{12345}, default_value{0};

    static constexpr auto mapper{[](T const &) { return 123.456f; }};

    static constexpr auto void_mapper{[](T const &) -> void {}};

    static void some() noexcept {
        Option<T> const option = kf::some(T{value});

        TEST_ASSERT_FALSE(option.isNone());
        TEST_ASSERT_TRUE(option.isSome());

        CHECK_CONSTRUCTOR_CALLS(T, 1);     // T{value}
        CHECK_DESTRUCTOR_CALLS(T, 1);      // temp T
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);//
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0); //
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);// move into Option
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0); //
    }

    static void none() noexcept {
        Option<T> const option = kf::none;

        TEST_ASSERT_TRUE(option.isNone());
        TEST_ASSERT_FALSE(option.isSome());

        CHECK_CONSTRUCTOR_CALLS(T, 0);
        CHECK_DESTRUCTOR_CALLS(T, 0);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void unwrap() noexcept {
        Option<T> const option = kf::some(T{value});

        TEST_ASSERT_TRUE(value == option.unwrap().value);

        CHECK_CONSTRUCTOR_CALLS(T, 1);
        CHECK_DESTRUCTOR_CALLS(T, 1);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void unwrap_or() noexcept {
        Option<T> const option = kf::none;

        TEST_ASSERT_TRUE(value == option.unwrapOr(T{value}).value);

        CHECK_CONSTRUCTOR_CALLS(T, 1);     // T{value}
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);// copy
        CHECK_DESTRUCTOR_CALLS(T, 2);      // temp T
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void reset() noexcept {
        Option<T> option = kf::some(T{value});
        option.reset();

        TEST_ASSERT_TRUE(option.isNone());

        CHECK_CONSTRUCTOR_CALLS(T, 1);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);
        CHECK_DESTRUCTOR_CALLS(T, 2);// temp + Option
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void map_some() noexcept {
        auto const option = kf::some(T{value}).map(mapper);

        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(option.unwrap() == mapper(T{value}));

        CHECK_CONSTRUCTOR_CALLS(T, 2);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);
        CHECK_DESTRUCTOR_CALLS(T, 3);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void void_map_some() noexcept {
        auto const option = kf::some(T{value}).map(void_mapper);

        TEST_ASSERT_TRUE(option.isSome());

        CHECK_CONSTRUCTOR_CALLS(T, 1);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);
        CHECK_DESTRUCTOR_CALLS(T, 2);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void map_none() noexcept {
        auto const option = Option<T>{kf::none}.map(mapper);

        TEST_ASSERT_TRUE(option.isNone());

        CHECK_CONSTRUCTOR_CALLS(T, 0);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 0);
        CHECK_DESTRUCTOR_CALLS(T, 0);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void copy() noexcept {
        auto original = kf::some(T{value});
        auto copy = original;

        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.unwrap().value);
        TEST_ASSERT_TRUE(value == copy.unwrap().value);
        TEST_ASSERT_FALSE(&original.unwrap().value == &copy.unwrap().value);

        CHECK_CONSTRUCTOR_CALLS(T, 1);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 1);
        CHECK_DESTRUCTOR_CALLS(T, 1);// temp T (original)
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void copy_assignment() noexcept {
        auto original = kf::some(T{value});
        Option<T> copy = kf::none;
        copy = original;

        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(value == original.unwrap().value);
        TEST_ASSERT_TRUE(value == copy.unwrap().value);

        CHECK_CONSTRUCTOR_CALLS(T, 1);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 1);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 1);
        CHECK_DESTRUCTOR_CALLS(T, 1);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void move() noexcept {
        auto original = kf::some(T{value});
        auto moved = std::move(original);

        TEST_ASSERT_TRUE(original.isNone());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == moved.unwrap().value);

        CHECK_CONSTRUCTOR_CALLS(T, 1);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 2);
        CHECK_DESTRUCTOR_CALLS(T, 2);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }

    static void move_assignment() noexcept {
        auto original = kf::some(T{value});
        Option<T> moved = kf::none;
        moved = std::move(original);

        TEST_ASSERT_TRUE(original.isNone());
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(value == moved.unwrap().value);

        CHECK_CONSTRUCTOR_CALLS(T, 1);
        CHECK_MOVE_CONSTRUCTOR_CALLS(T, 2);
        CHECK_DESTRUCTOR_CALLS(T, 2);
        CHECK_COPY_CONSTRUCTOR_CALLS(T, 0);
        CHECK_COPY_ASSIGNMENT_CALLS(T, 0);
        CHECK_MOVE_ASSIGNMENT_CALLS(T, 0);
    }
};

#define RESET_AND_RUN_TEST(__type__, __test__) \
    __type__::reset();                         \
    RUN_TEST(TestOption<__type__>::__test__)

#define RESET_AND_RUN_TEST_IF(__condition__, __type__, __test__) \
    if constexpr (__type__::__condition__) { RESET_AND_RUN_TEST(__type__, __test__); }

#define RUN_OPTION_TESTS(__type__)                              \
    RESET_AND_RUN_TEST(__type__, some);                         \
    RESET_AND_RUN_TEST(__type__, none);                         \
    RESET_AND_RUN_TEST(__type__, unwrap);                       \
    RESET_AND_RUN_TEST_IF(copyable, __type__, unwrap_or);       \
    RESET_AND_RUN_TEST(__type__, reset);                        \
    RESET_AND_RUN_TEST(__type__, map_some);                     \
    RESET_AND_RUN_TEST(__type__, void_map_some);                \
    RESET_AND_RUN_TEST(__type__, map_none);                     \
    RESET_AND_RUN_TEST_IF(copyable, __type__, copy);            \
    RESET_AND_RUN_TEST_IF(copyable, __type__, copy_assignment); \
    RESET_AND_RUN_TEST_IF(movable, __type__, move);             \
    RESET_AND_RUN_TEST_IF(movable, __type__, move_assignment);

void run_tests() {
    using namespace kf::test;

    RUN_OPTION_TESTS(TrivialType);
    RUN_OPTION_TESTS(OnlyMovable);
    RUN_OPTION_TESTS(OnlyCopyable);
    RUN_OPTION_TESTS(CopyableMovable);
}