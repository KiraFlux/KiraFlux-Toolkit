// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    ui/render/ColoredTextRenderer.hpp
/// @brief   Text renderer with semantic colours (gfx::Canvas codes).

#pragma once

#include "kf/Slice.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/DefaultResettable.hpp"

#include "kf/ui/Color.hpp"
#include "kf/ui/Layout.hpp"
#include "kf/ui/render/Renderer.hpp"
#include "kf/ui/render/TextualRenderer.hpp"

namespace kf::internal {

struct ColoredTextRendererConfig : mixin::DefaultResettable<ColoredTextRendererConfig> {

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

        [[nodiscard]] constexpr Item get(ui::Color color) const noexcept {
            switch (color) {
                case ui::Color::Normal: return normal;
                case ui::Color::Primary: return primary;
                case ui::Color::Secondary: return secondary;
                case ui::Color::Success: return success;
                case ui::Color::Warning: return warning;
                case ui::Color::Error: return error;// same value as ui::Color::Danger
                case ui::Color::Info: return info;
                case ui::Color::Disabled: return disabled;
                case ui::Color::Highlight: return highlight;
                default: return normal;
            }
        }
    };

    ui::render::TextualRenderer::Config textual{};

    Palette
        normal_foreground_palette{
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
        focused_foreground_palette{
            .normal = Palette::Black,
            .primary = Palette::Black,
            .secondary = Palette::Black,
            .success = Palette::Black,
            .warning = Palette::Black,
            .error = Palette::Black,
            .info = Palette::Black,
            .disabled = Palette::LightGray,
            .highlight = Palette::DarkPurple,
        },
        normal_background_palette{
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
        focused_background_palette{
            .normal = Palette::White,
            .primary = Palette::LightBlue,
            .secondary = Palette::LightGray,
            .success = Palette::LightGreen,
            .warning = Palette::LightYellow,
            .error = Palette::LightRed,
            .info = Palette::LightCyan,
            .disabled = Palette::LightGray,
            .highlight = Palette::LightPurple,
        };
};

};// namespace kf::internal

namespace kf::ui::render {

struct ColoredTextRenderer :

    Renderer<ColoredTextRenderer>,
    mixin::Configured<internal::ColoredTextRendererConfig>

{
    using Wrapped = TextualRenderer;
    using Config = internal::ColoredTextRendererConfig;

    explicit constexpr ColoredTextRenderer(Config const &config, Slice<char> source) noexcept :
        mixin::Configured<Config>{config}, _wrapped{config.textual, source} {}

    void callback(auto &&callback) noexcept {
        _wrapped.callback(std::forward<decltype(callback)>(callback));
    }

private:
    Wrapped _wrapped;
    bool _focus_active{false};

    void writeColor(Color color, char base_code, Config::Palette const &palette) noexcept {
        _wrapped.writeChar(base_code + static_cast<char>(palette.get(color)));
    }

    void writeForegroundColor(Color color, Config::Palette const &palette) noexcept {
        writeColor(color, '\xF0', palette);
    }

    void writeBackgroundColor(Color color, Config::Palette const &palette) noexcept {
        writeColor(color, '\xB0', palette);
    }

    KF_IMPL_RENDERER(ColoredTextRenderer);

    // control

    void beginFrameImpl() noexcept {
        _wrapped.beginFrame();
    }

    void endFrameImpl() noexcept {
        _wrapped.endFrame();
    }

    usize beginPageImpl(StringView title, Layout layout) noexcept {
        writeForegroundColor(Color::Normal, this->config().focused_foreground_palette);
        writeBackgroundColor(Color::Primary, this->config().focused_background_palette);
        return _wrapped.beginPage(title, layout);
    }

    void endPageImpl() noexcept {
        _wrapped.endPage();
    }

    void beginWidgetImpl(usize index, bool is_focused, usize offset) noexcept {
        _focus_active = is_focused;

        if (_wrapped.layout() == Layout::Vertical) {
            _wrapped.fillSpace(offset * this->config().textual.offset_size);
        }
    }

    void endWidgetImpl() noexcept {
        if (not _focus_active) { _wrapped.writeChar('\x80'); }
        _wrapped.endWidget();
        _focus_active = false;
    }

    void beginBlockImpl(Block block_type) noexcept {
        _wrapped.beginBlock(block_type);
    }

    void endBlockImpl(Block block_type) noexcept {
        _wrapped.endBlock(block_type);
    }

    void decorationImpl(Decoration decoration) noexcept {
        _wrapped.decoration(decoration);
    }

    void checkboxImpl(bool enabled) noexcept {
        this->background(enabled ? Color::Primary : Color::Secondary);
        _wrapped.checkbox(enabled);
    }

    void sliderImpl(f32 fill) noexcept {
        _wrapped.slider(fill);
    }

    // value

    void valueImpl(auto const &value) {
        _wrapped.value(value);
    }

    // semantic color

    void setForegroundImpl(Color color) noexcept {
        writeForegroundColor(color, _focus_active ? this->config().focused_foreground_palette : this->config().normal_foreground_palette);
    }

    void setBackgroundImpl(Color color) noexcept {
        writeBackgroundColor(color, _focus_active ? this->config().focused_background_palette : this->config().normal_background_palette);
    }

    // Properties

    constexpr Layout getLayoutImpl() const noexcept {
        return _wrapped.layout();
    }
};

}// namespace kf::ui::render