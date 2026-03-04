#include <kf/ui/Event.hpp>
#include <unity.h>

using namespace kf::ui;

template<kf::u8 V> struct EventTester {
    using Ev = Event<V>;
    using Value = typename Ev::Value;

    static void test_basic() {
        Value test_val = (V == 1) ? 0 : 1;
        Ev ev(Ev::Type::WidgetClick, test_val);
        TEST_ASSERT_EQUAL(Ev::Type::WidgetClick, ev.type());
        TEST_ASSERT_EQUAL(test_val, ev.value());
    }

    static void test_boundaries() {
        if constexpr (V > 1) {
            TEST_ASSERT(Ev::value_max > 0);
        }
        TEST_ASSERT(Ev::value_min < 0);

        Ev ev_min(Ev::Type::PageCursorMove, Ev::value_min);
        Ev ev_max(Ev::Type::PageCursorMove, Ev::value_max);
        TEST_ASSERT_EQUAL(Ev::value_min, ev_min.value());
        TEST_ASSERT_EQUAL(Ev::value_max, ev_max.value());
    }

    static void test_factories() {
        auto u = Ev::update();
        TEST_ASSERT_EQUAL(Ev::Type::Update, u.type());
        TEST_ASSERT_EQUAL(0, u.value());

        // Для V=1 используем 0, иначе 1
        Value move_val = (V == 1) ? 0 : 1;
        auto move = Ev::pageCursorMove(move_val);
        TEST_ASSERT_EQUAL(Ev::Type::PageCursorMove, move.type());
        TEST_ASSERT_EQUAL(move_val, move.value());

        auto click = Ev::widgetClick();
        TEST_ASSERT_EQUAL(Ev::Type::WidgetClick, click.type());
        TEST_ASSERT_EQUAL(0, click.value());

        Value widget_val = -1;
        auto val = Ev::widgetValue(widget_val);
        TEST_ASSERT_EQUAL(Ev::Type::WidgetValueChange, val.type());
        TEST_ASSERT_EQUAL(widget_val, val.value());
    }

    static void test_independence() {
        Ev ev(Ev::Type::WidgetValueChange, Ev::value_max);
        TEST_ASSERT_EQUAL(Ev::Type::WidgetValueChange, ev.type());
        TEST_ASSERT_EQUAL(Ev::value_max, ev.value());
    }

    static void test_zero() {
        Ev zero1(Ev::Type::Update, 0);
        Ev zero2(Ev::Type::WidgetClick, 0);
        TEST_ASSERT_EQUAL(0, zero1.value());
        TEST_ASSERT_EQUAL(0, zero2.value());
        TEST_ASSERT_EQUAL(Ev::Type::Update, zero1.type());
        TEST_ASSERT_EQUAL(Ev::Type::WidgetClick, zero2.type());
    }
};

#define RUN_EVENT_TESTS(V)                       \
    RUN_TEST(EventTester<V>::test_basic);        \
    RUN_TEST(EventTester<V>::test_boundaries);   \
    RUN_TEST(EventTester<V>::test_factories);    \
    RUN_TEST(EventTester<V>::test_independence); \
    RUN_TEST(EventTester<V>::test_zero)

int main() {
    UNITY_BEGIN();
    RUN_EVENT_TESTS(1);
    RUN_EVENT_TESTS(2);
    RUN_EVENT_TESTS(3);
    RUN_EVENT_TESTS(4);
    RUN_EVENT_TESTS(5);
    RUN_EVENT_TESTS(6);
    RUN_EVENT_TESTS(7);
    RUN_EVENT_TESTS(8);
    RUN_EVENT_TESTS(15);
    RUN_EVENT_TESTS(16);
    RUN_EVENT_TESTS(31);
    RUN_EVENT_TESTS(32);
    RUN_EVENT_TESTS(62);
    return UNITY_END();
}