#include <Arduino.h>

#include <kf/ui/Event.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/render/PlainTextRender.hpp>

// UI specialisation
using MyUI = kf::ui::UI<
    kf::ui::UiTraits<                        // Traits Implementation
        kf::ui::render::PlainTextRender<256>,// Render implementation: plain text, buffered (256 Bytes)
        kf::ui::Event<4>                     // Event type: 4-bit value
        >>;

using Event = MyUI::Traits::EventImpl;

static MyUI::Traits::RenderImpl::Config my_render_config{
    .row_max_length = 50,// console width = 50 chars
    .rows_total = 5,     // only 5 rows available (for scrolling)
    .float_places = 3,   // float rendering like:  1234.567
    .double_places = 6,  // double rendering like: 1.234567
    .title_centered = false,
};

static MyUI::Traits::RenderImpl my_render{
    my_render_config,// by ref
};

static MyUI my_ui{
    my_render,// by ref
};

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

    explicit MainPage() : Page{my_ui, "Main"} {
        widgets({widgets_storage.data(), widgets_storage.size()});

        click_button.callback([this]() {
            Serial.println("Test button clicked!");
            my_value += 1;
            value_display.value(my_value);
            update();// add Update Event
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

    explicit SettingsPage() : Page{my_ui, "Settings"} {
        widgets({widgets_storage.data(), widgets_storage.size()});

        ints_combo_box.callback([](int value) {
            Serial.print("Int Combo selected: ");
            Serial.println(value);
        });
        labeled_ints_combo_box.hint("Hint: this is int combo box for some this example");

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
Event eventFromChar(char c) {
    switch (c) {
        case 'w': return Event::pageCursorMove(-1);// Up
        case 's': return Event::pageCursorMove(+1);// Down
        case 'a': return Event::widgetValue(-1);   // Left
        case 'd': return Event::widgetValue(+1);   // Right
        case ' ': return Event::widgetClick();     // Click
        default: return Event::update();           // Other: Force update
    }
}

void setup() {
    Serial.begin(115200);

    // post-render procedure
    my_render.callback([](kf::memory::StringView text) {
        Serial.println("---");

        const auto &active_page = my_ui.activePage();
        if (active_page.isSome()) {
            const auto &selected_widget = active_page.unwrap().selectedWidget();

            if (selected_widget.isSome()) {
                Serial.println(selected_widget.unwrap().hint().data());
            }
        }

        Serial.print(text.data());
    });

    // insert navigation button on both pages
    main_page.widgets()[0] = &settings_page.link();
    settings_page.widgets()[0] = &main_page.link();

    my_ui.activePage(main_page);// start ui with main page

    my_ui.addEvent(Event::update());// Force update for first ui rendering
}

void loop() {
    if (Serial.available()) {
        const char c = Serial.read();
        my_ui.addEvent(eventFromChar(c));
    }

    const auto now = millis();
    my_ui.poll(now);

    delay(10);// 100 hz
}