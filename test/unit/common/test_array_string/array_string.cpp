#include <cmath>
#include <kf/memory/ArrayString.hpp>
#include <unity.h>

using kf::memory::ArrayString;
using kf::memory::StringView;

template<kf::usize N> static void assertStr(const ArrayString<N> &s, const char *exp, size_t len) {
    TEST_ASSERT_EQUAL_UINT32(len, s.size());
    if (len) {
        TEST_ASSERT_EQUAL_MEMORY(exp, s.data(), len);
    }
    TEST_ASSERT_EQUAL_UINT8(0, s.data()[len]);
}

namespace constructors {
void empty() {
    ArrayString<10> s;
    TEST_ASSERT_EQUAL(0, s.size());
    TEST_ASSERT_TRUE(s.empty());
}
void literal() {
    ArrayString<10> s("hello");
    assertStr(s, "hello", 5);
}
void truncated() {
    ArrayString<10> s("too long string");
    assertStr(s, "too long s", 10);
}
void formatted() {
    auto s = ArrayString<10>::formatted("num=%d", 42);
    assertStr(s, "num=42", 6);
}
}// namespace constructors

namespace access {
void index() {
    ArrayString<10> s{"abc"};
    TEST_ASSERT_EQUAL_CHAR('b', s[1]);
    s[1] = 'x';
    TEST_ASSERT_EQUAL_CHAR('x', s[1]);
    assertStr(s, "axc", 3);
}
void view() {
    ArrayString<10> s("test");
    StringView v = s;
    TEST_ASSERT_EQUAL(4, v.size());
    TEST_ASSERT_EQUAL_MEMORY("test", v.data(), 4);
}
void conversion() {
    ArrayString<10> s("hello");
    const char *p = s.data();
    TEST_ASSERT_EQUAL_STRING("hello", p);
}
}// namespace access

namespace push_pop {
void sequence() {
    ArrayString<5> s;
    for (char c = 'a'; c <= 'e'; c++) {
        TEST_ASSERT_TRUE(s.push(c));
    }

    TEST_ASSERT_FALSE(s.push('f'));
    assertStr(s, "abcde", 5);

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_TRUE(s.pop());
    }

    TEST_ASSERT_FALSE(s.pop());
    TEST_ASSERT_TRUE(s.empty());
}
}// namespace push_pop

namespace append {
void string() {
    ArrayString<10> s;
    TEST_ASSERT_EQUAL(5, s.append("world"));
    TEST_ASSERT_EQUAL(3, s.append(" !!"));
    assertStr(s, "world !!", 8);
}
void integer() {
    ArrayString<10> s;
    TEST_ASSERT_EQUAL(1, s.append(0));
    assertStr(s, "0", 1);
    s.clear();
    TEST_ASSERT_EQUAL(5, s.append(-1234));
    assertStr(s, "-1234", 5);
}
void floating() {
    ArrayString<10> s;
    TEST_ASSERT_EQUAL(4, s.append(3.1415, 2));
    assertStr(s, "3.14", 4);
    s.clear();
    TEST_ASSERT_EQUAL(3, s.append(NAN, 2));
    assertStr(s, "nan", 3);
    s.clear();
    TEST_ASSERT_EQUAL(3, s.append(INFINITY, 2));
    assertStr(s, "inf", 3);
}
void integer_overflow() {
    ArrayString<5> s;
    s = "1234";
    TEST_ASSERT_EQUAL(0, s.append(-5));
    assertStr(s, "1234", 4);
}
void float_overflow() {
    ArrayString<5> s;
    s = "1234";
    TEST_ASSERT_EQUAL(0, s.append(3.14, 2));
    assertStr(s, "1234", 4);
}
}// namespace append

namespace insert {
void beginning() {
    ArrayString<10> s("world");
    TEST_ASSERT_EQUAL(5, s.insert(0, "hello "));
    assertStr(s, "helloworld", 10);
}
void middle() {
    ArrayString<10> s("heloworld");
    TEST_ASSERT_EQUAL(1, s.insert(3, "l"));
    assertStr(s, "helloworld", 10);
}
void full() {
    ArrayString<10> s("1234567890");
    TEST_ASSERT_EQUAL(0, s.insert(5, "x"));
    assertStr(s, "1234567890", 10);
}
}// namespace insert

namespace erase {
void beginning() {
    ArrayString<12> s("hello world");
    TEST_ASSERT_EQUAL(6, s.erase(0, 6));
    assertStr(s, "world", 5);
}
void middle() {
    ArrayString<12> s("hello world");
    TEST_ASSERT_EQUAL(4, s.erase(3, 4));
    assertStr(s, "helorld", 7);
}
void all() {
    ArrayString<10> s("hello");
    TEST_ASSERT_EQUAL(5, s.erase(0, 10));
    TEST_ASSERT_TRUE(s.empty());
}
}// namespace erase

namespace format {
void simple() {
    ArrayString<10> s;
    const auto result = s.format("hello");
    TEST_ASSERT_TRUE(result.isOk());
    TEST_ASSERT_EQUAL(5, result.value());
    assertStr(s, "hello", 5);
}
void truncated() {
    ArrayString<10> s;
    (void) s.format("this is a very long string__________________________");
    assertStr(s, "this is a ", 10);
}
}// namespace format

namespace trim {
void start() {
    ArrayString<20> s("   hello");
    s.trimStart();
    assertStr(s, "hello", 5);
}
void end() {
    ArrayString<20> s("hello   ");
    s.trimEnd();
    assertStr(s, "hello", 5);
}
void both() {
    ArrayString<20> s("   hello world   ");
    s.trim();
    assertStr(s, "hello world", 11);
}
}// namespace trim

namespace search {
void find_char() {
    ArrayString<20> s("hello world");
    auto p = s.find('o');
    TEST_ASSERT_TRUE(p.isSome() && p.value() == 4);
    p = s.find('x');
    TEST_ASSERT_FALSE(p.isSome());
}
void find_str() {
    ArrayString<20> s("hello world");
    auto p = s.find("world");
    TEST_ASSERT_TRUE(p.isSome() && p.value() == 6);
}
void starts_ends() {
    ArrayString<20> s("hello.txt");
    TEST_ASSERT_TRUE(s.startsWith("hello"));
    TEST_ASSERT_TRUE(s.endsWith(".txt"));
}
void compare() {
    ArrayString<10> a("apple"), b("apple");
    TEST_ASSERT_EQUAL(0, a.compare(b.view()));
    TEST_ASSERT_TRUE(a.compare("banana") != 0);
    TEST_ASSERT_TRUE(a.compare("banana") < 0);
}
}// namespace search

namespace assign {
void from_string() {
    ArrayString<10> s;
    s.assign("test");
    assertStr(s, "test", 4);
}
void from_view() {
    ArrayString<10> s;
    s = StringView("hello");
    assertStr(s, "hello", 5);
}
}// namespace assign

namespace operators {
void equality() {
    ArrayString<10> a("apple"), b("apple"), c("banana");
    TEST_ASSERT_TRUE(a == b);
    TEST_ASSERT_FALSE(a == c);
    TEST_ASSERT_TRUE(a != c);
}
}// namespace operators

int main() {
    UNITY_BEGIN();
    RUN_TEST(constructors::empty);
    RUN_TEST(constructors::literal);
    RUN_TEST(constructors::truncated);
    RUN_TEST(constructors::formatted);
    RUN_TEST(access::index);
    RUN_TEST(access::view);
    RUN_TEST(access::conversion);
    RUN_TEST(push_pop::sequence);
    RUN_TEST(append::string);
    RUN_TEST(append::integer);
    RUN_TEST(append::integer_overflow);
    RUN_TEST(append::float_overflow);
    RUN_TEST(append::floating);
    RUN_TEST(insert::beginning);
    RUN_TEST(insert::middle);
    RUN_TEST(insert::full);
    RUN_TEST(erase::beginning);
    RUN_TEST(erase::middle);
    RUN_TEST(erase::all);
    RUN_TEST(format::simple);
    RUN_TEST(format::truncated);
    RUN_TEST(trim::start);
    RUN_TEST(trim::end);
    RUN_TEST(trim::both);
    RUN_TEST(search::find_char);
    RUN_TEST(search::find_str);
    RUN_TEST(search::starts_ends);
    RUN_TEST(search::compare);
    RUN_TEST(assign::from_string);
    RUN_TEST(assign::from_view);
    RUN_TEST(operators::equality);
    return UNITY_END();
}