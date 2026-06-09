#pragma once

#include "kf/ui/Color.hpp"
#include "kf/ui/Placement.hpp"
#include "kf/ui/render/PlainTextRender.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui::render {

template<usize N> struct ColoredTextRender : Render<ColoredTextRender<N>> {
    using Wrapped = PlainTextRender<N>;
    using Config = typename Wrapped::Config;

    explicit constexpr ColoredTextRender(const Config &config) noexcept : _wrapped{config} {}

    template<typename F> void callback(F &&callback) noexcept { _wrapped.callback(std::forward<F>(callback)); }

private:
    Wrapped _wrapped{};

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
        static constexpr char foreground_table[8] = {
            '\xFF',// Normal -> bright white
            '\xFC',// Primary -> bright blue
            '\xF7',// Secondary -> white
            '\xFA',// Success -> bright green
            '\xFB',// Warning -> bright yellow
            '\xF9',// Error -> bright red
            '\xFE',// Info -> bright cyan
            '\xF8',// Disabled -> bright black
        };
        _wrapped.writeChar(foreground_table[static_cast<char>(color)]);
    }

    void setBackgroundImpl(Color color) noexcept {
        static constexpr char background_table[8] = {
            '\xB0',// Normal -> black
            '\xBC',// Primary -> bright blue
            '\xB8',// Secondary -> dim black
            '\xB2',// Success -> green
            '\xB3',// Warning -> yellow
            '\xB1',// Error -> red
            '\xB6',// Info -> cyan
            '\xB7',// Disabled -> white
        };
        _wrapped.writeChar(background_table[static_cast<char>(color)]);
    }

    // decoration

    void arrowImpl() noexcept { _wrapped.arrow(); }

    void colonImpl() noexcept { _wrapped.colon(); }

    void beginFocusedImpl() noexcept {
        _wrapped.writeChar('\x81');
    }

    void endFocusedImpl() noexcept {
        _wrapped.endFocused();
        _wrapped.writeChar('\x80');
    }

    void beginBlockImpl() noexcept {
        _wrapped.beginBlock();
    }

    void endBlockImpl() noexcept { _wrapped.endBlock(); }

    void beginAltBlockImpl() noexcept { _wrapped.beginAltBlock(); }

    void endAltBlockImpl() noexcept { _wrapped.endAltBlock(); }

    void beginWidgetImpl(usize index) noexcept { _wrapped.beginWidget(index); }

    void endWidgetImpl() noexcept { 
        _wrapped.writeChar('\x80');
        _wrapped.endWidget(); 
    }
};

}// namespace kf::ui::render