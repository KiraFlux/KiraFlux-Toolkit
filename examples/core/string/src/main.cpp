// KiraFlux-Toolkit Demo 'string'
#include <Arduino.h>

#include <kf/Slice.hpp>
#include <kf/String.hpp>

void setup() {
    Serial.begin(115200);

    char buffer[256];
    kf::String my_string{{buffer}};// construct from slice from array

    // formatting: use {} as anchor, {{ or }} to display '{' or '}'
    my_string.format(

        "{{ hello }} {} {} {} {} {} {} {}\n",
        kf::StringView{"string-view"},
        "c-string",
        123,  // integer
        6.7,  // float-point
        true, // boolean: true
        false,// boolean: false
        'H'   // char

    );

    Serial.write(my_string.data(), my_string.length());

    Serial.printf(

        "Length/Capacity: %d/%d\n",
        my_string.length(), // current length
        my_string.capacity()// maximal length

    );

    // clear (just set head to zero)
    my_string.reset();

    // append

    // any Sequence<char>
    my_string.append(kf::StringView{"sv"});
    my_string.append("c-str");
    // my_string.append();

    // booleans
    my_string.append(true);
    my_string.append(false);

    // numbers
    my_string.append(kf::i64{13456789});
    my_string.append(0.4 - 0.3, 50);// precision

    // char
    my_string.append('c');

    // from Stack<char>
    const bool ok = my_string.write('c');
    const kf::Option<char> popped = my_string.read();

    // convert to c-style string - add terminating null
    Serial.print(my_string.cString());

    delay(5000);
}

void loop() {}