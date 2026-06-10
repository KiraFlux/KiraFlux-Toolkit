#pragma once

#include "kf/ui/Color.hpp"
#include "kf/ui/Placement.hpp"
#include "kf/ui/render/PlainTextRender.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui::render {

template<usize N> struct ColoredTextRender : Render<ColoredTextRender<N>> {
    using Wrapped = PlainTextRender<N>;
    using Config = typename Wrapped::Config;

    enum class ASCII : u8 {
        Black = 0x00,
        DarkRed = 0x01,
        DarkGreen = 0x02,
        DarkYellow = 0x03,
        DarkBlue = 0x04,
        DarkPurple = 0x05,
        DarkCyan = 0x06,
        LightGray = 0x07,
        DarkGray = 0x08,
        LightRed = 0x09,
        LightGreen = 0x0A,
        LightYellow = 0x0B,
        LightBlue = 0x0C,
        LightPurple = 0x0D,
        LightCyan = 0x0E,
        White = 0x0F,
    };

    explicit constexpr ColoredTextRender(const Config &config) noexcept : _wrapped{config} {}

    template<typename F> void callback(F &&callback) noexcept { _wrapped.callback(std::forward<F>(callback)); }

private:
    static constexpr ASCII normal_foreground_codes_table[8]{
        ASCII::White,      // Normal
        ASCII::LightBlue,  // Primary
        ASCII::LightGray,  // Secondary
        ASCII::LightGreen, // Success
        ASCII::LightYellow,// Warning
        ASCII::LightRed,   // Error
        ASCII::LightCyan,  // Info
        ASCII::DarkGray,   // Disabled
    };

    static constexpr ASCII focused_foreground_codes_table[8]{
        ASCII::Black,    // Normal
        ASCII::Black,    // Primary
        ASCII::Black,    // Secondary
        ASCII::Black,    // Success
        ASCII::Black,    // Warning
        ASCII::Black,    // Error
        ASCII::Black,    // Info
        ASCII::LightGray,// Disabled
    };

    static constexpr ASCII normal_background_codes_table[8]{
        ASCII::Black,     // Normal
        ASCII::LightBlue, // Primary
        ASCII::DarkGray,  // Secondary
        ASCII::DarkGreen, // Success
        ASCII::DarkYellow,// Warning
        ASCII::DarkRed,   // Error
        ASCII::DarkCyan,  // Info
        ASCII::LightGray, // Disabled
    };

    static constexpr ASCII focused_background_codes_table[8]{
        ASCII::White,      // Normal
        ASCII::DarkBlue,   // Primary
        ASCII::LightGray,  // Secondary
        ASCII::LightGreen, // Success
        ASCII::LightYellow,// Warning
        ASCII::LightRed,   // Error
        ASCII::LightCyan,  // Info
        ASCII::DarkGray,   // Disabled
    };

    Wrapped _wrapped{};
    bool _coloring_active{false};
    bool _focus_active{false};

    void writeColor(Color color, char base_code, const ASCII *code_from_color_table) noexcept {
        _wrapped.writeChar(base_code + static_cast<char>(code_from_color_table[static_cast<u8>(color)]));
        _coloring_active = true;
    }

    KF_IMPL(Render<ColoredTextRender<N>>);

    // control

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept { return _wrapped.widgetsAvailable(); }

    void beginFrameImpl() noexcept { _wrapped.beginFrame(); }

    void endFrameImpl() noexcept { _wrapped.endFrame(); }

    void titleImpl(memory::StringView title) noexcept {
        _wrapped.writeChar('\xF0');
        _wrapped.writeChar('\xBC');
        _wrapped.title(title);
        _wrapped.writeChar('\x80');
    }

    void beginWidgetImpl(usize index, bool focused) noexcept {
        _focus_active = focused;
        this->foreground(Color::Normal);
        this->background(Color::Normal);
    }

    void endWidgetImpl() noexcept {
        if (_focus_active) { _wrapped.endWidget(); }

        if (_coloring_active) {
            _wrapped.writeChar('\x80');
            _coloring_active = false;
        }

        if (not _focus_active) { _wrapped.endWidget(); }

        _focus_active = false;
    }

    void checkboxImpl(bool enabled) noexcept {
        this->background(enabled ? Color::Primary : Color::Secondary);
        _wrapped.checkbox(enabled);
    }

    template<typename T> void sliderImpl(const T &value, const Range<T> &range, Placement placement) noexcept {
        _wrapped.slider(value, range, placement);
    }

    // value

    template<typename T> void valueImpl(const T &v) { _wrapped.value(v); }

    // semantic color

    void setForegroundImpl(Color color) noexcept {
        writeColor(color, '\xF0', _focus_active ? focused_foreground_codes_table : normal_foreground_codes_table);
    }

    void setBackgroundImpl(Color color) noexcept {
        writeColor(color, '\xB0', _focus_active ? focused_background_codes_table : normal_background_codes_table);
    }

    // decoration

    void arrowImpl() noexcept { _wrapped.arrow(); }

    void colonImpl() noexcept { _wrapped.colon(); }

    void beginBlockImpl() noexcept { _wrapped.beginBlock(); }

    void endBlockImpl() noexcept { _wrapped.endBlock(); }

    void beginAltBlockImpl() noexcept { _wrapped.beginAltBlock(); }

    void endAltBlockImpl() noexcept { _wrapped.endAltBlock(); }
};

}// namespace kf::ui::render