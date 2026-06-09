#include <Arduino.h>

#include <kf/ui/Event.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/render/PlainTextRender.hpp>

// UI specialisation
using MyUI = kf::ui::UI<
    kf::ui::render::PlainTextRender<256>,// Render implementation: plain text, buffered (256 Bytes)
    kf::ui::Event<4>                     // Event type: 4-bit value
    >;

// User-defined example pages

struct MainPage : MyUI::Page {

    int my_value{12345};

    MyUI::Button click_button{
        "Test",// button label
    };

    MyUI::CheckBox check_box{
        true,// default: true
    };

    MyUI::Display<int> value_display{
        my_value,// initial value
    };

    using MySlider = MyUI::Slider<int>;

    MySlider::Config slider_config{
        .value_range = {
            .start = 0,
            .end = 1000,
        },
        .default_value = 0,
        .step = 25,
        .placement = kf::ui::Placement::Outside,
        .init_show_value = true,
    };

    MySlider slider{
        slider_config,// by ref
    };

    kf::memory::Array<MyUI::Widget *, 5> widgets_storage{
        {
            nullptr,// link widget will be init in setup()
            &click_button,
            &check_box,
            &value_display,
            &slider,
        },
    };

    explicit MainPage() : Page{"Main"} {
        widgets({widgets_storage.data(), widgets_storage.size()});

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

    using PresetInput = MyUI::ComboBox<int>;

    kf::memory::Array<PresetInput::Item, 3> ints_combo_box_items{
        {
            {/* label: */ "Normal", /* value: int */ 100},
            {"Sport", 200},
            {"Quiet", 20},
        },// initializer list
    };

    PresetInput::Config ints_combo_box_config{
        .items = {ints_combo_box_items.data(), ints_combo_box_items.size()},// Slice
    };

    PresetInput ints_combo_box{
        ints_combo_box_config,// by ref
    };

    MyUI::Labeled labeled_ints_combo_box{
        "Preset",      // label
        ints_combo_box,// by ref
    };

    using MyCombo = MyUI::ComboBox<kf::memory::StringView>;

    kf::memory::Array<MyCombo::Item, 3> strings_combo_box_items{
        {"Alpha", "Beta", "Gamma"},// StringView-typed combo item implicit constructs from string literal
    };

    MyCombo::Config strings_combo_box_config{
        .items = {strings_combo_box_items.data(), strings_combo_box_items.size()},// Slice
    };

    MyCombo strings_combo_box{
        strings_combo_box_config,// by ref
    };

    using MySpinBox = MyUI::SpinBox<int, MyUI::Traits::GeometricAdjuster<int>>;

    MySpinBox::Config spin_box_config{
        .default_step = 2,
        .step_adjust = 2,
    };

    MySpinBox spin_box{
        spin_box_config,// ref
        10,             // = default value
    };

    kf::memory::Array<MyUI::Widget *, 4> widgets_storage{
        {
            nullptr,// link widget will be init in setup()
            &labeled_ints_combo_box,
            &strings_combo_box,
            &spin_box,
        },
    };

    explicit SettingsPage() : Page{"Settings"} {
        widgets({widgets_storage.data(), widgets_storage.size()});

        ints_combo_box.callback([](int value) {
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
MyUI::Traits::EventImpl eventFromChar(char c) {
    switch (c) {
        case 'w': return MyUI::Traits::EventImpl::pageCursorMove(-1);// Up
        case 's': return MyUI::Traits::EventImpl::pageCursorMove(+1);// Down
        case 'a': return MyUI::Traits::EventImpl::widgetValue(-1);   // Left
        case 'd': return MyUI::Traits::EventImpl::widgetValue(+1);   // Right
        case ' ': return MyUI::Traits::EventImpl::widgetClick();     // Click
        default: return MyUI::Traits::EventImpl::update();           // Other: Force update
    }
}

static auto &ui = MyUI::instance();

void setup() {
    Serial.begin(115200);

    // render setup
    auto &config = ui.renderConfig();

    // post-render procedure
    ui.renderSystem().callback([](kf::memory::StringView text) {
        Serial.println("---");
        Serial.print(text.data());
    });

    // misc
    config.float_places = 3;   // float rendering like:  1234.567
    config.double_places = 6;  // double rendering like: 1.234567
    config.rows_total = 5;     // only 5 rows available (for scrolling)
    config.row_max_length = 50;// console width = 50 chars

    // insert navigation button on both pages
    main_page.widgets()[0] = &settings_page.link();
    settings_page.widgets()[0] = &main_page.link();

    ui.bindPage(main_page);// start ui with main page

    ui.addEvent(MyUI::Traits::EventImpl::update());// Force update for first ui rendering
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