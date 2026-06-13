// Demo: setup UI with colored textual render system into hardware display

#include <Arduino.h>

// uses in this demo:
#include <kf/bus/spi/ArduinoSPI.hpp>
#include <kf/drivers/display/Orientation.hpp>
#include <kf/drivers/display/ST7735.hpp>
#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/gpio/ArduinoGPIO.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/ui/Event.hpp>
#include <kf/ui/Style.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/render/ColoredTextRender.hpp>
#include <kf/ui/widgets/Widget.hpp>

#include <kf/Option.hpp>

using kf::bus::spi::ArduinoSPI;
using kf::drivers::display::Orientation;
using kf::gpio::ArduinoGPIO;

// Display Driver specialisation
using MyDisplayDriver = kf::drivers::display::ST7735<ArduinoSPI::Node, ArduinoGPIO::DigitalOutput>;
using P = MyDisplayDriver::PixelImpl;// shortcut for pixel impl

// UI specialisation
using MyUI = kf::ui::UI<
    kf::ui::UiTraits<                              // Traits Implementation
        kf::ui::widgets::Widget<                   // Base widget class
            kf::ui::render::ColoredTextRender<256>,// Render implementation: plain text, buffered (256 Bytes),
            kf::ui::Event<4>                       // Event type: 4-bit value
            >>>;

// shortcusts
using Event = MyUI::Traits::EventImpl;
using Render = MyUI::Traits::RenderImpl;
using Color = kf::ui::Color;
using Style = kf::ui::Style;

static Render::Config my_render_config{Render::Config::defaults()};// will set in setup

static Render my_render{
    my_render_config,// by ref
};

static MyUI my_ui{
    my_render,// by ref
};

// User-defined example pages

struct MainPage : MyUI::Page {

    using MyValueType = kf::TrivialOption<int>;

    MyValueType my_value{kf::someTrivial(12345)};

    MyUI::Button click_button{
        "Button",// button label

        // style setup (all widget has style as last parameter)

        // kf::ui::Style{
        //     .foreground_color = kf::ui::Color::Primary,
        //     .background_color = kf::ui::Color::Primary,
        // }
    };

    MyUI::CheckBox check_box{
        true,// default: true
    };

    MyUI::Display<MyValueType> value_display{
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

    using ColorCombo = MyUI::ComboBox<Color>;

    kf::memory::Array<ColorCombo::Config::Item, 9> color_combo_items{{
        {"Normal", Color::Normal},
        // combo option implements Styled
        {
            "Primary",
            Color::Primary,
            Style{
                .foreground_color = Color::Primary,
                // .background_color = Color::Secondary,
            },
        },
        {
            "Secondary",
            Color::Secondary,
            Style{
                // .foreground_color = Color::Primary,
                .background_color = Color::Secondary,
            },
        },
        {"Success", Color::Success},
        {"Warning", Color::Warning},
        {"Error", Color::Error},
        {"Info", Color::Info},
        {"Disabled", Color::Disabled},
        {"Highlight", Color::Highlight},
    }};

    ColorCombo::Config color_combo_config{
        .items = {color_combo_items.data(), color_combo_items.size()},
    };

    ColorCombo foreground_color_combo{color_combo_config};
    ColorCombo background_color_combo{color_combo_config};

    MyUI::Labeled labeled_foreground_color_combo{"FG", foreground_color_combo};
    MyUI::Labeled labeled_background_color_combo{"BG", background_color_combo};
    MyUI::Labeled labeled_check_box{"Check Box", check_box};

    kf::memory::Array<MyUI::Widget *, 7> widgets_storage{
        {
            nullptr,// link widget will be init in setup()
            &click_button,
            &labeled_foreground_color_combo,
            &labeled_background_color_combo,
            &labeled_check_box,
            &value_display,
            &slider,
        },
    };

    explicit MainPage() : Page{my_ui, "Main"} {
        widgets({widgets_storage.data(), widgets_storage.size()});

        click_button.callback([this]() {
            Serial.println("Test button clicked!");

            if (my_value.isSome()) {
                my_value.unwrap() += 1;
                value_display.value(my_value);
                update();// add Update Event
            }
        });

        // style
        const auto style = click_button.style();
        click_button.style(Style{
            .foreground_color = kf::ui::Color::Normal,
            .background_color = kf::ui::Color::Normal,
        });
        click_button.foreground(kf::ui::Color::Normal);
        const auto fg = click_button.foreground();
        click_button.background(kf::ui::Color::Normal);
        const auto bg = click_button.background();

        check_box.callback([this](bool state) {
            Serial.print("Checkbox changed to: ");
            Serial.println(state ? "ON" : "OFF");

            if (my_value.isSome()) {
                my_value = kf::none;
            } else {
                my_value = kf::someTrivial(12345);
            }

            value_display.value(my_value);
        });

        background_color_combo.callback([this](auto color) {
            click_button.background(color);
        });

        foreground_color_combo.callback([this](auto color) {
            click_button.foreground(color);
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
    void onPoll(kf::math::Milliseconds now) noexcept override {}

} main_page{};

struct SettingsPage : MyUI::Page {

    using PresetInput = MyUI::ComboBox<int>;

    kf::memory::Array<PresetInput::Config::Item, 3> ints_combo_box_items{
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

    kf::memory::Array<MyCombo::Config::Item, 3> strings_combo_box_items{{
        // StringView-typed combo item implicit constructs from string literal
        "Alpha",
        "Beta",
        {
            "Gamma",
            Style{
                .foreground_color = Color::Highlight,
            },
        },
    }};

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
        case 'd': return Event::widgetValue(1);    // Right
        case ' ': return Event::widgetClick();     // Click
        default: return Event::update();           // Other: Force update
    }
}

static auto bus_config{ArduinoSPI::Config::create()};// use defauls

static ArduinoSPI bus{
    bus_config,
    SPI,
};

static auto node_config{
    ArduinoSPI::Node::Config::create(
        GPIO_NUM_5,// CS
        27'000'000 // SPI frequency
        ),
};

// display config
static MyDisplayDriver::Config display_config{
    .init_orientation = Orientation::ClockWise,
};

// Driver instance references config and SPI bus.
static MyDisplayDriver display{
    display_config,
    bus.createNode(node_config),
    ArduinoGPIO::DigitalOutput{GPIO_NUM_22},// DC
    ArduinoGPIO::DigitalOutput{GPIO_NUM_17},// RESET
};

// display

void setup() {
    Serial.begin(115200);

    (void) bus.init();

    // display setup
    if (not display.init()) {
        Serial.println("Failed to init display. halting...");

        while (true) {}
    }

    // render setup
    using CanvasImpl = kf::gfx::Canvas<P>;

    static CanvasImpl root_canvas{
        kf::image::DynamicImage<P>{display.image()},
        CanvasImpl::State{
            .active_font = kf::someRef(kf::gfx::fonts::gyver_5x7_en),
            .foreground_color = CanvasImpl::PaletteType::white,
            .background_color = CanvasImpl::PaletteType::black,
            .auto_next_line = true,
        },
    };

    // post-render procedure
    my_render.callback([](kf::memory::StringView text) {
        root_canvas.fill();
        root_canvas.text(0, 0, text);

        (void) display.send();// SPI cannot tell anything about error => ignoring

        // show buffer content
        for (auto c: text) {
            if (c < 0x80) {
                Serial.write(c);
            } else {
                Serial.write(' ');
                Serial.print(c, 16);
            }
        }
    });

    // render config setup
    my_render_config.text.float_places = 3;                         // float rendering like:  1234.567
    my_render_config.text.double_places = 6;                        // double rendering like: 1.234567
    my_render_config.text.rows_total = root_canvas.heightInGlyphs();// all canvas area
    my_render_config.text.row_max_length = root_canvas.widthInGlyphs();

    // my_render_config.normal_foreground_palette.normal = Render::Config::Palette::Black; // style configutation
    // my_render_config.focused_foreground_palette
    // my_render_config.normal_background_palette
    // my_render_config.focused_background_palette
    // fields mapping UI Semantic Color: normal, primary, secondary, success, warning, error, info, disabled

    // insert navigation button on both pages
    main_page.widgets()[0] = &settings_page.link();
    settings_page.widgets()[0] = &main_page.link();

    my_ui.activePage(main_page);    // start ui with main page
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