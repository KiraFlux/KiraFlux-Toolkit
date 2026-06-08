#pragma once

#include "kf/ui/Placement.hpp"
#include "kf/ui/render/PlainTextRender.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui::render {

template<usize N> struct ColoredTextRender : Render<ColoredTextRender<N>> {
    using Wrapped = PlainTextRender<N>;
    using Config = typename Wrapped::Config;

    Wrapped wrapped{};

    explicit constexpr ColoredTextRender(const Config &config) noexcept : wrapped{config} {}

private:
    KF_IMPL(Render<ColoredTextRender<N>>);

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept { return wrapped.widgetsAvailable(); }

    void prepareImpl() noexcept { wrapped.prepare(); }

    void finishImpl() noexcept { wrapped.finish(); }

    void titleImpl(memory::StringView title) noexcept {
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

    template<typename T> void sliderImpl(T value, Range<T> value_range, Placement value_placement) noexcept {
        wrapped.slider(value, value_range, value_placement);
    }

    template<typename T> void valueImpl(T v) { wrapped.value(v); }

    void arrowImpl() noexcept { wrapped.arrow(); }

    void colonImpl() noexcept { wrapped.colon(); }

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

    void beginWidgetImpl(usize index) noexcept { wrapped.beginWidget(index); }

    void endWidgetImpl() noexcept { wrapped.endWidget(); }
};

}// namespace kf::ui::render