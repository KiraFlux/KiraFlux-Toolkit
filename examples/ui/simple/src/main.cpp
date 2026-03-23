#include <Arduino.h>

#include <kf/ui/Event.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/render/PlainTextRender.hpp>

// UI specialisation
using MyUI = kf::ui::UI<
    kf::ui::render::PlainTextRender<256>,// Render implementation: plain text, buffered (256 Bytes)
    kf::ui::Event<4>                     // Event type: 4-bit value
    >;

// User-defined page examples

struct MainPage : MyUI::Page {

    int my_value{12345};

    MyUI::Button click_button{
        *this, // add to this page
        "Test",// button label
    };

    MyUI::CheckBox check_box{
        *this,// add to this page
        true, // default: true
    };

    MyUI::Display<int> value_display{
        *this,   // add to this page
        my_value,// initial value
    };

    MyUI::Slider<int> slider{
        *this,// add to this page
        50,   // initial value
        5,    // step
    };

    explicit MainPage() : Page{"Main"} {
        click_button.callback([this]() {
            Serial.println("Test button clicked!");
            my_value += 1;
            value_display.value(my_value);
        });

        check_box.callback([this](bool state) {
            Serial.print("Checkbox changed to: ");
            Serial.println(state ? "ON" : "OFF");
            my_value *= -1;
            value_display.value(my_value);
        });

        slider.max_value = 100;
        slider.min_value = 0;
        slider.placement = MyUI::ValuePlacement::Inside;
    }

    // Page virtual methods

    // behavior on entry
    void onEntry() noexcept override {
        Serial.println("Entry on Main");
    }

    // behavior on leave
    void onExit() noexcept override {
        Serial.println("Exit from Main");
    }

    // behavior on UI polling
    void onUpdate(kf::math::Milliseconds now) noexcept override {}

} main_page{};

struct SettingsPage : MyUI::Page {

    using PresetInput = MyUI::Labeled<MyUI::ComboBox<int, 3>>;

    PresetInput labeled_ints_combo_box{
        *this,   // attach to this page
        "Preset",// label
        PresetInput::Wrapped{
            {{{"Normal", 100}, {"Sport", 200}, {"Quiet", 20}}}// items
        },                                                    // spinbox
    };

    MyUI::ComboBox<kf::memory::StringView, 3> strings_combo_box{
        *this,                     // attach to this page
        {"Alpha", "Beta", "Gamma"},// items (3)
    };

    MyUI::SpinBox<int, MyUI::StepMode::Arithmetic> spin_box{
        *this,// attach to this page
        10,   // = default value
        1,    // = step
    };

    explicit SettingsPage() : Page{"Settings"} {
        labeled_ints_combo_box.wrapped.callback([](int value) {
            Serial.print("Int Combo selected: ");
            Serial.println(value);
        });

        strings_combo_box.callback([](kf::memory::StringView value) {
            Serial.print("String Combo selected: ");
            Serial.println(value.data());
        });

        spin_box.callback([](int value) {
            Serial.print("SpinBox value: ");
            Serial.println(value);
        });
    }
} settings_page{};

// Simple function for convertion from char to event
MyUI::Event eventFromChar(char c) {
    switch (c) {
        case 'w': return MyUI::Event::pageCursorMove(-1);// Up
        case 's': return MyUI::Event::pageCursorMove(+1);// Down
        case 'a': return MyUI::Event::widgetValue(-1);   // Left
        case 'd': return MyUI::Event::widgetValue(+1);   // Right
        case ' ': return MyUI::Event::widgetClick();     // Click
        default: return MyUI::Event::update();           // Other: Force update
    }
}

static auto &ui = MyUI::instance();

void setup() {
    Serial.begin(115200);

    // render setup
    MyUI::RenderConfig &config = ui.renderConfig();

    // post-render procedure
    config.on_render_finish = [](kf::memory::StringView text) {
        Serial.println("---");
        Serial.print(text.data());
    };

    // misc
    config.float_places = 3;   // float rendering like:  1234.567
    config.double_places = 6;  // double rendering like: 1.234567
    config.rows_total = 5;     // only 5 rows available (for scrolling)
    config.row_max_length = 50;// console width = 50 chars

    main_page.link(settings_page);// add special navigation button on both pages
    ui.bindPage(main_page);       // start ui with main page

    ui.addEvent(MyUI::Event::update());// Force update for first ui rendering
}

void loop() {
    if (Serial.available()) {
        const char c = Serial.read();
        ui.addEvent(eventFromChar(c));
    }

    const auto now = millis();
    ui.poll(now);

    delay(20);// 50 hz
}