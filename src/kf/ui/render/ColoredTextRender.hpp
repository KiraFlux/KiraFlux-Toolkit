#pragma once

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
        _wrapped.writeChar(enabled ? '\xB2' : '\xB1');
        _wrapped.checkbox(enabled);
        _wrapped.writeChar('\x80');
    }

    template<typename T> void sliderImpl(const T &value, const Range<T> &range, Placement placement) noexcept {
        _wrapped.slider(value, range, placement);
    }

    template<typename T> void valueImpl(const T &v) { _wrapped.value(v); }

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