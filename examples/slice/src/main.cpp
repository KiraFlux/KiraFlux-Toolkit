#include <Arduino.h>
#include <kf/memory/Slice.hpp>

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println(F("=== Slice Example ==="));

    int numbers[] = {10, 20, 30, 40, 50};
    kf::Slice<int> slice{numbers, 5};

    kf::Slice<int> slice_from_array{numbers};

    // element access
    Serial.print(F("slice[2] = "));
    Serial.println(slice[2]);

    // modification
    slice[2] = 99;
    Serial.print(F("numbers[2] = "));
    Serial.println(numbers[2]);

    // manual iteration with it += 1
    Serial.print(F("Elements (manual): "));
    for (auto it = slice.begin(); it != slice.end(); it += 1) {
        Serial.print(*it);
        Serial.print(' ');
    }
    Serial.println();

    // range-based for
    Serial.print(F("Elements (range):  "));
    for (int v: slice) {
        Serial.print(v);
        Serial.print(' ');
    }
    Serial.println();

    // first / last / sub
    auto firstTwo = slice.first(2);
    auto lastTwo = slice.last(2);
    auto middle = slice.sub(1, 3);

    Serial.print(F("first(2): "));
    for (int v: firstTwo) {
        Serial.print(v);
        Serial.print(' ');
    }
    Serial.println();

    Serial.print(F("last(2):  "));
    for (int v: lastTwo) {
        Serial.print(v);
        Serial.print(' ');
    }
    Serial.println();

    Serial.print(F("sub(1,3): "));
    for (int v: middle) {
        Serial.print(v);
        Serial.print(' ');
    }
    Serial.println();

    // fromOffset
    auto from2 = slice.fromOffset(2);
    Serial.print(F("fromOffset(2): "));
    for (int v: from2) {
        Serial.print(v);
        Serial.print(' ');
    }
    Serial.println();

    // const slice
    const int constData[] = {7, 8, 9};
    kf::Slice<const int> constSlice{constData, 3};
    Serial.print(F("const slice: "));
    for (int v: constSlice) {
        Serial.print(v);
        Serial.print(' ');
    }
    Serial.println();

    // different types
    double doubleData[] = {1.1, 2.2, 3.3};
    kf::Slice<double> doubleSlice{doubleData, 3};
    Serial.print(F("double slice: "));
    for (double v: doubleSlice) {
        Serial.print(v, 1);
        Serial.print(' ');
    }
    Serial.println();

    // empty slice
    kf::Slice<int> empty{numbers, 0};
    Serial.print(F("empty slice size: "));
    Serial.println(empty.size());

    Serial.println(F("=== End ==="));
}

void loop() {}