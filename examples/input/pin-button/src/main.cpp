// KiraFlux-Toolkit Demo 'input-button'
#include <Arduino.h>

#include <kf/gpio/arduino.hpp>
#include <kf/input/Button.hpp>

using kf::gpio::arduino::DigitalInput;
using Button = kf::input::Button<DigitalInput>;

Button::Config my_button_config{
    .debounce = 50,// ms
};

Button my_button{/* & */ my_button_config, DigitalInput{GPIO_NUM_15, DigitalInput::Pull::InternalDown}};

void setup() {
    Serial.begin(115200);

    my_button.init();// setup GPIO
}

void loop() {
    my_button.poll(millis());

    if (my_button.clicked()) {
        Serial.println("click");
    }

    // const bool is_pressed = my_button.pressed(); // debounce-proof check

    delay(1);
}
