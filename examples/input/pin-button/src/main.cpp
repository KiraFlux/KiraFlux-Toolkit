// KiraFlux-Toolkit Demo 'input-button'
#include <Arduino.h>

#include <kf/gpio/ArduinoGPIO.hpp>
#include <kf/input/LogicalLevelListener.hpp>

using kf::gpio::ArduinoGPIO;
using Button = kf::input::LogicalLevelListener<ArduinoGPIO>;

Button::Config my_button_config{
    .debounce = 50,// ms
};

Button my_button{
    /* &: */
    my_button_config,
    /* move: */
    ArduinoGPIO::DigitalInput{
        GPIO_NUM_15,
        ArduinoGPIO::DigitalInput::Pull::InternalDown,
    },
};

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
