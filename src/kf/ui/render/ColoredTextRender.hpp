#pragma once

#include "kf/ui/internal/ValuePlacement.hpp"
#include "kf/ui/render/PlainTextRender.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui::render {

template<usize N> struct ColoredTextRender : Render<ColoredTextRender<N>> {
    using Wrapped = PlainTextRender<N>;
    using Config = typename Wrapped::Config;

    constexpr explicit ColoredTextRender(const Config &config) noexcept : _wrapped{config} {}

private:
    Wrapped _wrapped{};

    KF_IMPL(Render<ColoredTextRender<N>>);

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept { return _wrapped.widgetsAvailable(); }

    void prepareImpl() noexcept { _wrapped.prepare(); }

    void finishImpl() noexcept { _wrapped.finish(); }

    void titleImpl(memory::StringView title) noexcept {
        _wrapped.writeChar('\xF0');
        _wrapped.writeChar('\xBC');
        _wrapped.title(title);
        _wrapped.writeChar('\x80');
    }

    void checkboxImpl(bool enabled) noexcept {
        _wrapped.writeChar(enabled ? '\xB2' : '\xB1');
        _wrapped.checkbox(enabled);
        _wrapped.writeChar('\x80');
    }

    template<typename T> void sliderImpl(
        T value, T min_value, T max_value,
        ui::internal::ValuePlacement value_placement) noexcept {
        _wrapped.splider(value, min_value, max_value, value_placement);
    }

    template<typename T> void valueImpl(T v) { _wrapped.value(v); }

    void arrowImpl() noexcept { _wrapped.arrow(); }

    void colonImpl() noexcept { _wrapped.colon(); }

    void beginFocusedImpl() noexcept {
        _wrapped.writeChar('\x81');
    }

    void endFocusedImpl() noexcept {
        _wrapped.endFocused();
        _wrapped.writeChar('\x80');
    }

    void beginBlockImpl() noexcept { _wrapped.beginBlock(); }

    void endBlockImpl() noexcept { _wrapped.endBlock(); }

    void beginAltBlockImpl() noexcept { _wrapped.beginAltBlock(); }

    void endAltBlockImpl() noexcept { _wrapped.endAltBlock(); }

    void beginWidgetImpl(usize index) noexcept { _wrapped.beginWidget(index); }

    void endWidgetImpl() noexcept { _wrapped.endWidget(); }
};

}// namespace kf::ui::render
