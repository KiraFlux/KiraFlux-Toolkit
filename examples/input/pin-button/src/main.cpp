// KiraFlux-Toolkit Demo 'input-button'
#include <Arduino.h>

#include <kf/arduino/ArduinoGPIO.hpp>
#include <kf/listener/LogicalLevelListener.hpp>

using DigitalInput = kf::arduino::ArduinoGPIO::DigitalInput;
using Button = kf::listener::LogicalLevelListener;

Button::Config my_button_listener_config{
    .debounce = 50,// ms
};

DigitalInput my_button_gpio{
    GPIO_NUM_25,
    DigitalInput::Pull::InternalUp,
};

Button my_button_listener{
    my_button_listener_config,// capture by reference
};

void setup() {
    Serial.begin(115200);

    my_button_gpio.init();

    my_button_listener.callback([]() {
        Serial.println("click");
    });
}

void loop() {
    my_button_listener.set(my_button_gpio.read());
    my_button_listener.poll(millis());

    delay(1);
}