#pragma once

#include "kf/ui/render/PlainTextRender.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui {

template<usize N> struct ColoredTextRender : Render<ColoredTextRender<N>> {
    friend struct Render<ColoredTextRender<N>>;

private:
    using Wrapped = PlainTextRender<N>;
    Wrapped wrapped{};

public:
    using Config = typename Wrapped::Config;

    Config &getConfig() { return wrapped.getConfig(); }

private:
    // Render CRTP Implementation

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept { return wrapped.widgetsAvailable(); }

    void prepareImpl() noexcept { wrapped.prepare(); }

    void finishImpl() noexcept { wrapped.finish(); }

    void titleImpl(StringView title) noexcept {
        wrapped.writeChar('\xF0');
        wrapped.writeChar('\xBC');
        wrapped.title(title);
        wrapped.writeChar('\x80');
    }

    void checkboxImpl(bool enabled) noexcept {
        wrapped.writeChar(enabled ? '\xB2' : '\xB1');
        wrapped.checkbox(enabled);
        wrapped.writeChar('\x80');
    }

    template<typename T> valueImpl(T value) { wrapped.value(value); }

    void arrowImpl() noexcept { wrapped.arrow(); }

    void colonImpl() noexcept { wrapped.colon() }

    void beginFocusedImpl() noexcept {
        wrapped.writeChar('\x81');
    }

    void endFocusedImpl() noexcept {
        wrapped.endFocused();
        wrapped.writeChar('\x80');
    }

    void beginBlockImpl() noexcept { wrapped.beginBlock(); }

    void endBlockImpl() noexcept { wrapped.endBlock(); }

    void beginAltBlockImpl() noexcept { wrapped.beginAltBlock(); }

    void endAltBlockImpl() noexcept { wrapped.endAltBlock(); }

    void beginWidgetImpl(usize) noexcept { wrapped.beginWidget(); }

    void endWidgetImpl() noexcept { wrapped.endWidget(); }
};

}// namespace kf::ui
