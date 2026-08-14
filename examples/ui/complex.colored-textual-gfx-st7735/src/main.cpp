// Demo: setup UI with colored textual render system into hardware display

// App
#include <kf/Logger.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Clock.hpp>
#include <kf/rtos/Task.hpp>

// Algorithm
#include <kf/Array.hpp>

// Hardware
#include <kf/bus/SPI.hpp>
#include <kf/driver/display/Orientation.hpp>
#include <kf/driver/display/ST7735.hpp>
#include <kf/gpio.hpp>

// Graphics stack
#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/image/DynamicImage.hpp>

// UI stack
#include <kf/ui/Event.hpp>
#include <kf/ui/Style.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/render/ColoredTextRenderer.hpp>
#include <kf/ui/widget/Widget.hpp>

using kf::driver::display::Orientation;

// Display Driver specialization
using MyDisplayDriver = kf::driver::display::ST7735;
using P = MyDisplayDriver::PixelImpl;// shortcut for pixel impl

// UI specialization
using MyUI = kf::ui::UI<

    // Traits Implementation
    kf::ui::UiTraits<

        // Base widget class
        kf::ui::widget::Widget<

            // Renderer implementation: plain text
            kf::ui::render::ColoredTextRenderer,
            // Event type: 4-bit value
            kf::ui::Event<4>
            //
            >
        //
        >
    //
    >;

// shortcuts
using Event = MyUI::Traits::EventImpl;
using Render = MyUI::Traits::RendererImpl;
using Color = kf::ui::Color;
using Style = kf::ui::Style;

// User-defined example pages

struct MainPage : MyUI::Page {

    using MyValueType = kf::Option<int>;

    MyValueType my_value{12345};

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
        .init_show_value = true,
    };

    MySlider slider{
        slider_config,// by ref
    };

    using ColorCombo = MyUI::ComboBox<Color>;

    kf::Array<ColorCombo::Config::Item, 9> color_combo_items{
        .items = {
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
        },
    };

    ColorCombo::Config color_combo_config{
        .items = color_combo_items.slice(),
    };

    ColorCombo foreground_color_combo{color_combo_config};
    ColorCombo background_color_combo{color_combo_config};

    MyUI::Labeled labeled_foreground_color_combo{"FG", foreground_color_combo};
    MyUI::Labeled labeled_background_color_combo{"BG", background_color_combo};
    MyUI::Labeled labeled_check_box{"Check Box", check_box};

    using Display = MyUI::Display<kf::u8>;

    static constexpr auto regular_widgets{7}, array_widgets{20};

    // Many widgets for scroll check
    kf::Array<Display, array_widgets> displays{};

    kf::Array<MyUI::Widget *, (regular_widgets + array_widgets)> widgets_storage{
        .items = {
            nullptr,// link widget will be init in setup()
            &click_button,
            &labeled_foreground_color_combo,
            &labeled_background_color_combo,
            &labeled_check_box,
            &value_display,
            &slider,
        },
    };

    inline static kf::Logger logger{"main"};

    explicit MainPage(MyUI &ui) : Page{ui /* layout = kf::ui::Layout::Vertical */} {
        this->label("Main");

        for (auto i = 0u; i < array_widgets; i += 1) {
            auto &d = displays[i];

            widgets_storage[i + regular_widgets] = &d;
            d.value(i);
            d.background((i % 2 == 0) ? Color::Secondary : Color::Primary);
        }

        click_button.callback([this]() {
            logger.info("Test button clicked!");

            if (my_value.isSome()) {
                my_value.unwrap() += 1;
                value_display.value(my_value);
                _ui.requestRender();// add Update Event
            }
        });

        // style
        auto const style = click_button.style();
        click_button.style(Style{
            .foreground_color = kf::ui::Color::Normal,
            .background_color = kf::ui::Color::Normal,
        });
        click_button.foreground(kf::ui::Color::Normal);
        auto const fg = click_button.foreground();
        click_button.background(kf::ui::Color::Normal);
        auto const bg = click_button.background();

        check_box.callback([this](bool state) {
            logger.info("Checkbox changed to: {}", state ? "ON" : "OFF");

            if (my_value.isSome()) {
                my_value = kf::none;
            } else {
                my_value = kf::some(12345);
            }

            value_display.value(my_value);

            requestBuild();
        });

        background_color_combo.callback([this](auto item) {
            click_button.background(item.value());
        });

        foreground_color_combo.callback([this](auto item) {
            click_button.foreground(item.value());
        });
    }

    // Page virtual methods

    WidgetsView build() noexcept override {
        auto widgets = widgets_storage.slice();
        // cutoff generated widgets if checkbox set on
        return check_box.value() ? widgets : widgets.first(regular_widgets);
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

    using MyCombo = MyUI::ComboBox<kf::ui::Layout>;

    kf::Array<MyCombo::Config::Item, 2> layout_combo_box_items{
        .items = {
            {
                "Vertical",
                kf::ui::Layout::Vertical,
            },
            {
                "Horizontal",
                kf::ui::Layout::Horizontal,
                Style{
                    .foreground_color = Color::Highlight,
                },
            },
        },
    };

    MyCombo::Config layout_combo_box_config{
        .items = layout_combo_box_items.slice(),
    };

    MyCombo layout_combo_box{
        layout_combo_box_config,// by ref
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

    kf::Array<MyUI::Widget *, 3> widgets_storage{
        .items = {
            nullptr,// link widget will be init in setup()
            &layout_combo_box,
            &spin_box,
        },
    };

    inline static kf::Logger logger{"settings"};

    explicit SettingsPage(MyUI &ui) : Page{ui} {
        this->label("Settings");

        layout_combo_box.callback([this](MyCombo::Config::Item item) {
            this->layout(item.value());
            logger.info("Combo selected: {}", item.label());
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
Event eventFromChar(char c) {
    switch (c) {
        case 'w': return Event::pageCursorMove(-1);// Up
        case 's': return Event::pageCursorMove(+1);// Down
        case 'a': return Event::widgetValue(-1);   // Left
        case 'd': return Event::widgetValue(1);    // Right
        case ' ':
        default:
            return Event::widgetClick();// Click
    }
}

// display

void kf::main(kf::Init &init) {
    static Render::Config my_renderer_config{
        .textual{
            .offset_size = 1,
        },
    };

    static Render my_renderer{
        my_renderer_config,// by ref
        init.arena.allocate<char>(256),
    };

    static MyUI my_ui{
        init.arena.allocate<MyUI::Traits::EventImpl>(64),
        my_renderer,// by ref
    };

    static MainPage main_page{my_ui};
    static SettingsPage settings_page{my_ui};

    static bus::SPI::Config const spi_bus_config{
        // use defaults
        .gpio_num_mosi = none,
        .gpio_num_miso = none,
        .gpio_num_sck = none,
    };

    static bus::SPI spi_bus{spi_bus_config};

    // display config
    static MyDisplayDriver::Config driver_config{
        // Node configuration
        .spi_node = {
            .clock_hz = 27'000'000,
            .gpio_num_cs = gpio::G5,
            .bit_order = bus::SPI::Node::Config::BitOrder::MostSignificant,
            .clock_bits = bus::SPI::Node::Config::ClockBits::None,
        },

        .init_orientation = Orientation::ClockWise,
    };

    // Driver instance (static: SHOULD NOT lay on stack cuz buffer image is 40KiB)
    static MyDisplayDriver display{
        spi_bus,      // used to create node
        driver_config,// by const reference
        gpio::G22,    // DC
        gpio::G17,    // RESET
    };

    (void) spi_bus.init();

    // display setup
    if (display.init().isError()) {
        init.logger.error("Failed to init display. halting...");
        return;
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
    my_renderer.callback([](kf::StringView text) {
        root_canvas.fill();
        root_canvas.text(0, 0, text);

        (void) display.send();// SPI cannot tell anything about error => ignoring

        // TODO: hex formatting {x}
        // // show buffer content
        // for (auto c: text) {
        //     if (c < 0x80) {
        //         Serial.write(c);
        //     } else {
        //         Serial.write(' ');
        //         Serial.print(c, 16);
        //     }
        // }
    });

    // render config reset to defaults
    my_renderer_config.reset();

    // override
    my_renderer_config.textual.float_places = 3;                         // float rendering like:  1234.567
    my_renderer_config.textual.double_places = 6;                        // double rendering like: 1.234567
    my_renderer_config.textual.rows_total = root_canvas.heightInGlyphs();// all canvas area
    my_renderer_config.textual.row_max_length = root_canvas.widthInGlyphs();

    // my_render_config.normal_foreground_palette.normal = Renderer::Config::Palette::Black; // style configuration
    // my_render_config.focused_foreground_palette
    // my_render_config.normal_background_palette
    // my_render_config.focused_background_palette
    // fields mapping UI Semantic Color: normal, primary, secondary, success, warning, error, info, disabled

    // insert navigation button on both pages
    main_page.widgets_storage[0] = &settings_page.link();
    settings_page.widgets_storage[0] = &main_page.link();

    my_ui.activePage(main_page);// start ui with main page
    my_ui.requestRender();      // Force update for first ui rendering

    while (true) {

        // process all incoming keys into UI event queue
        while (init.io.availableForRead()) {
            if (auto const read = init.io.readByte(); read.isOk()) {
                my_ui.addEvent(eventFromChar(read.ok()));
            }
        }

        my_ui.poll(rtos::Clock::now());

        rtos::Task::sleep(10);// 100 hz
    }
}