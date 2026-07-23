#include <kf/main.hpp>

#include <kf/Array.hpp>
#include <kf/Logger.hpp>
#include <kf/rtos/Clock.hpp>
#include <kf/rtos/Task.hpp>

#include <kf/ui/Event.hpp>
#include <kf/ui/Style.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/render/TextualRenderer.hpp>
#include <kf/ui/widget/Widget.hpp>

using MyEvent = kf::ui::Event<4>;// Event type: 4-bit value

// UI specialization
using MyUI = kf::ui::UI<
    kf::ui::UiTraits<                       // Traits Implementation
        kf::ui::widget::Widget<             // Base widget class
            kf::ui::render::TextualRenderer,// Renderer implementation: plain text
            MyEvent>>>;

// User-defined example pages

struct MainPage : MyUI::Page {

    inline static kf::Logger logger{"main"};

    int my_value{12345};

    MyUI::Button click_button{
        "Test",// button label
        // setup style (all widget has style as last parameter)
        // kf::ui::Style{
        //     .foreground_color = kf::ui::Color::Primary,
        //     .background_color = kf::ui::Color::Primary,
        // }
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
        .init_show_value = true,
    };

    MySlider slider{
        slider_config,// by ref
    };

    kf::Array<MyUI::Widget *, 5> widgets_storage{.items{
        nullptr,// link widget will be init in setup()
        &click_button,
        &check_box,
        &value_display,
        &slider,
    }};

    explicit MainPage(MyUI &ui) : Page{ui} {
        this->label("Main");

        click_button.callback([this]() {
            logger.info("Test button clicked!");
            my_value += 1;
            value_display.value(my_value);
            _ui.requestRender();
        });

        check_box.callback([this](bool state) {
            logger.info("Checkbox changed to: {}", state ? "ON" : "OFF");
            my_value *= -1;
            value_display.value(my_value);
        });
    }

    // Page virtual methods

    WidgetsView build() noexcept override {
        return widgets_storage.slice();
    }

    // behavior on entry
    void onEntry() noexcept override {
        logger.debug("Entry on Main");
    }

    // behavior on leave
    void onExit() noexcept override {
        logger.debug("Exit from Main");
    }

protected:
    // behavior on UI polling
    void onPoll(kf::units::Milliseconds now) noexcept override {}
};

struct SettingsPage : MyUI::Page {

    inline static kf::Logger logger{"setting"};

    using PresetInput = MyUI::ComboBox<int>;

    kf::Array<PresetInput::Config::Item, 3> ints_combo_box_items{.items{
        {"Normal", 100},
        {"Sport", 200},
        {"Quiet", 20},
    }};

    PresetInput::Config ints_combo_box_config{
        .items = ints_combo_box_items.slice(),
    };

    PresetInput ints_combo_box{
        ints_combo_box_config,// by ref
    };

    MyUI::Labeled labeled_ints_combo_box{
        "Preset",      // label
        ints_combo_box,// by ref
    };

    using MyCombo = MyUI::ComboBox<kf::StringView>;

    kf::Array<MyCombo::Config::Item, 3> strings_combo_box_items{.items{
        // StringView-typed combo item implicit constructs from string literal
        "Alpha",
        "Beta",
        "Gamma",
    }};

    MyCombo::Config strings_combo_box_config{
        .items = strings_combo_box_items.slice(),
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

    kf::Array<MyUI::Widget *, 4> widgets_storage{.items{
        nullptr,// link widget will be init in setup()
        &labeled_ints_combo_box,
        &strings_combo_box,
        &spin_box,
    }};

    explicit SettingsPage(MyUI &ui) : Page{ui} {
        this->label("Settings");

        ints_combo_box.callback([](auto item) {
            logger.info("Int Combo selected: {}", item.value());
        });
        labeled_ints_combo_box.hint("Hint: this is int combo box for some this example");

        strings_combo_box.callback([](auto item) {
            logger.info("String Combo selected: {}", item.value().data());
        });

        spin_box.callback([](int value) {
            logger.info("SpinBox value: {}", value);
        });
    }

    WidgetsView build() noexcept override {
        return widgets_storage.slice();
    }
};

// Simple function for conversion from char to event
MyEvent eventFromChar(char c) {
    switch (c) {
        case 'w': return MyEvent::pageCursorMove(-1);// Up
        case 's': return MyEvent::pageCursorMove(+1);// Down
        case 'a': return MyEvent::widgetValue(-1);   // Left
        case 'd': return MyEvent::widgetValue(+1);   // Right
        case ' ':
        default:
            return MyEvent::widgetClick();// Click
    }
}

void kf::main(kf::Init &init) {
    // allocate memory for buffers
    static char my_renderer_buffer[256]{};
    static char my_event_buffer[64 * sizeof(MyUI::Traits::EventImpl)];

    MyUI::Traits::RendererImpl::Config my_renderer_config{
        .row_max_length = 50,// console width = 50 chars
        .rows_total = 5,     // only 5 rows available (for scrolling)
        .float_places = 3,   // float rendering like:  1234.567
        .double_places = 6,  // double rendering like: 1.2345670
        .title_centered = false,
    };

    MyUI::Traits::RendererImpl my_renderer{
        my_renderer_config,// by ref
        {my_renderer_buffer},
    };

    MyUI my_ui{
        {reinterpret_cast<MyUI::Traits::EventImpl *>(my_event_buffer), sizeof(my_event_buffer)},
        my_renderer,// by ref
    };

    SettingsPage settings_page{my_ui};
    MainPage main_page{my_ui};

    // post-render procedure
    my_renderer.callback([&init, &my_ui](kf::StringView text) {
        init.logger.info("UI: {}\n", text);

        auto const &active_page = my_ui.activePage();
        if (active_page.isSome()) {
            auto const &selected_widget = active_page.unwrap().selectedWidget();

            if (selected_widget.isSome()) {
                init.logger.info("hint: {}", selected_widget.unwrap().hint().data());
            }
        }
    });

    // insert navigation button on both pages
    main_page.widgets_storage[0] = &settings_page.link();
    settings_page.widgets_storage[0] = &main_page.link();

    my_ui.activePage(main_page);// start ui with main page
    my_ui.requestRender();      // Force update for first ui rendering

    while (true) {

        while (init.io.availableForRead() > 0) {
            if (auto const read = init.io.readByte(); read.isOk()) {
                my_ui.addEvent(eventFromChar(read.ok()));
            }
        }

        my_ui.poll(rtos::Clock::now());

        rtos::Task::sleep(10);
    }
}