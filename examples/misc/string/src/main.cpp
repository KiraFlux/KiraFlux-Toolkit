// KiraFlux-Toolkit Demo 'string'
#include <Arduino.h>

#include <kf/Slice.hpp>
#include <kf/String.hpp>

void setup() {
    Serial.begin(115200);

    char buffer[100];

    kf::Slice<char> my_buffer_view{buffer};

    kf::String my_string{my_buffer_view};

    my_string.format("{{ hello }} {}", 123);
    
    Serial.write(my_string.data(), my_string.length());
}

void loop() {}