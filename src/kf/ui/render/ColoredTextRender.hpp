// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Configurable.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/Placement.hpp"
#include "kf/ui/render/PlainTextRender.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::internal {

template<usize N> struct ColoredTextRenderConfig final {
    using TextConfig = typename ui::render::PlainTextRender<N>::Config;

    struct Palette final {

        /// @brief Semantic colors
        enum Item : u8 {
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

        Item normal, primary, secondary, success, warning, error, info, disabled, highlight;

        constexpr Item get(ui::Color color) const noexcept {
            return reinterpret_cast<const Item *>(this)[static_cast<char>(color)];
        }
    };

    TextConfig text;
    Palette normal_foreground_palette, focused_foreground_palette, normal_background_palette, focused_background_palette;

    [[nodiscard]] static constexpr auto defaults() noexcept {
        return ColoredTextRenderConfig{
            .text = TextConfig::defaults(),
            .normal_foreground_palette = {
                .normal = Palette::White,
                .primary = Palette::LightBlue,
                .secondary = Palette::LightGray,
                .success = Palette::LightGreen,
                .warning = Palette::LightYellow,
                .error = Palette::LightRed,
                .info = Palette::LightCyan,
                .disabled = Palette::DarkGray,
                .highlight = Palette::LightPurple,
            },
            .focused_foreground_palette = {
                .normal = Palette::Black,
                .primary = Palette::Black,
                .secondary = Palette::Black,
                .success = Palette::Black,
                .warning = Palette::Black,
                .error = Palette::Black,
                .info = Palette::Black,
                .disabled = Palette::LightGray,
                .highlight = Palette::Black,
            },
            .normal_background_palette = {
                .normal = Palette::Black,
                .primary = Palette::DarkBlue,
                .secondary = Palette::DarkGray,
                .success = Palette::DarkGreen,
                .warning = Palette::DarkYellow,
                .error = Palette::DarkRed,
                .info = Palette::DarkCyan,
                .disabled = Palette::DarkGray,
                .highlight = Palette::DarkPurple,
            },
            .focused_background_palette = {
                .normal = Palette::White,
                .primary = Palette::LightBlue,
                .secondary = Palette::LightGray,
                .success = Palette::LightGreen,
                .warning = Palette::LightYellow,
                .error = Palette::LightRed,
                .info = Palette::LightCyan,
                .disabled = Palette::LightGray,
                .highlight = Palette::LightPurple,
            },
        };
    }
};

};// namespace kf::internal

namespace kf::ui::render {

template<usize N> struct ColoredTextRender :

    Render<ColoredTextRender<N>>,
    mixin::Configurable<internal::ColoredTextRenderConfig<N>>

{
    using Wrapped = PlainTextRender<N>;
    using Config = internal::ColoredTextRenderConfig<N>;

    explicit constexpr ColoredTextRender(const Config &config) noexcept :
        mixin::Configurable<Config>{config}, _wrapped{config.text} {}

    template<typename F> void callback(F &&callback) noexcept {
        _wrapped.callback(std::forward<F>(callback));
    }

private:
    Wrapped _wrapped;
    bool _focus_active{false};

    void writeColor(Color color, char base_code, const typename Config::Palette &palette) noexcept {
        _wrapped.writeChar(base_code + static_cast<char>(palette.get(color)));
    }

    void writeForegroundColor(Color color, const typename Config::Palette &palette) noexcept {
        writeColor(color, '\xF0', palette);
    }

    void writeBackgroundColor(Color color, const typename Config::Palette &palette) noexcept {
        writeColor(color, '\xB0', palette);
    }

    KF_IMPL(Render<ColoredTextRender<N>>);

    // control

    [[nodiscard]] usize widgetsAvailableImpl() const noexcept {
        return _wrapped.widgetsAvailable();
    }

    void beginFrameImpl() noexcept {
        _wrapped.beginFrame();
    }

    void endFrameImpl() noexcept {
        _wrapped.endFrame();
    }

    void titleImpl(memory::StringView title) noexcept {
        writeForegroundColor(Color::Normal, this->config().focused_foreground_palette);
        writeBackgroundColor(Color::Primary, this->config().focused_background_palette);
        _wrapped.title(title);
    }

    void beginWidgetImpl(usize index, bool is_focused, Color foreground, Color background) noexcept {
        _focus_active = is_focused;
        this->foreground(foreground);
        this->background(background);
    }

    void endWidgetImpl() noexcept {
        if (not _focus_active) { _wrapped.writeChar('\x80'); }
        _wrapped.endWidget();
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

    template<typename T> void valueImpl(const T &value) {
        _wrapped.value(value);
    }

    // semantic color

    void setForegroundImpl(Color color) noexcept {
        writeForegroundColor(color, _focus_active ? this->config().focused_foreground_palette : this->config().normal_foreground_palette);
    }

    void setBackgroundImpl(Color color) noexcept {
        writeBackgroundColor(color, _focus_active ? this->config().focused_background_palette : this->config().normal_background_palette);
    }

    // decoration

    void arrowImpl() noexcept {
        _wrapped.arrow();
    }

    void colonImpl() noexcept {
        _wrapped.colon();
    }

    void beginBlockImpl() noexcept {
        _wrapped.beginBlock();
    }

    void endBlockImpl() noexcept {
        _wrapped.endBlock();
    }

    void beginAltBlockImpl() noexcept {
        _wrapped.beginAltBlock();
    }

    void endAltBlockImpl() noexcept {
        _wrapped.endAltBlock();
    }
};

}// namespace kf::ui::render