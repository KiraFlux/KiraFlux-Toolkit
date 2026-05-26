#include <Arduino.h>
#include <kf/Slice.hpp>

using kf::Slice;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println(F("=== Slice Demo ==="));

    int raw[] = {10, 20, 30, 40, 50};
    Slice<int> s{raw, 5}; // from pointer+size
    Slice<int> s_arr{raw};// from array (deduced size)

    Serial.print(F("s[2]="));
    Serial.print(s[2]);// 30
    s[2] = 99;         // modification
    Serial.print(F(" -> s[2]="));
    Serial.println(s[2]);

    // iteration
    Serial.print(F("elements: "));
    for (auto x: s) {
        Serial.print(x);
        Serial.print(' ');
    }
    Serial.println();

    // sub‑slices
    auto first = s.first(2);
    auto last = s.last(2);
    auto mid = s.sub(1, 3);
    auto from2 = s.fromOffset(2);

    auto print = [](auto slice, const char *tag) {
        Serial.print(tag);
        Serial.print(": ");
        for (auto x: slice) {
            Serial.print(x);
            Serial.print(' ');
        }
        Serial.println();
    };
    print(first, "first(2)");
    print(last, "last(2)");
    print(mid, "sub(1,3)");
    print(from2, "fromOffset(2)");

    // const‑correctness
    const int carr[] = {7, 8, 9};
    Slice<const int> cs{carr, 3};// const slice
    // cs[1] = 0; // error
    print(cs, "const");

    // conversion to const slice
    Slice<const int> cs2 = s;// operator Slice<const T>
    print(cs2, "converted");

    // other types
    double dbl[] = {1.1, 2.2, 3.3};
    Slice<double> ds{dbl, 3};
    print(ds, "double");

    // empty slice
    Slice<int> empty{raw, 0};
    Serial.print(F("empty.size()="));
    Serial.println(empty.size());

    Serial.println(F("=== End ==="));
}

void loop() {}