// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Labeled.hpp"

#include "kf/ui/Color.hpp"
#include "kf/ui/Decoration.hpp"
#include "kf/ui/Request.hpp"
#include "kf/ui/Style.hpp"

namespace ui::widget {

/// @brief Collapsible section: click to show/hide child widgets
/// @note Stores a view of child widgets; returns them only when expanded.
/// @note Click toggles expansion, changes background color, and requests a page rebuild (so the widget list changes).
/// @tparam U UI Traits Type
template<typename U> struct Fold :

    U::Widget,
    mixin::Labeled

{
    using WidgetsView = Slice<typename U::Widget *>;

    static constexpr Color
        expanded_background_color{Color::Primary},
        folded_background_color{Color::Secondary};

    explicit constexpr Fold(WidgetsView widgets, StringView label = {}) noexcept :
        U::Widget{Style{.background_color = folded_background_color}}, mixin::Labeled{label}, _widgets{widgets} {}

    /// @brief Get Fold widgets
    /// @returns Fold widgets if expanded, empty view if collapsed
    [[nodiscard]] constexpr WidgetsView widgets() const noexcept {
        return _expanded ? _widgets : WidgetsView{};
    }

    /// @brief Set Fold widgets
    constexpr void widgets(WidgetsView new_widgets) noexcept {
        _widgets = new_widgets;
    }

    /// @brief Is Fold expanded
    [[nodiscard]] constexpr bool expanded() const noexcept {
        return _expanded;
    }

    /// @brief Set Fold expanded
    constexpr void expanded(bool is_expanded) const noexcept {
        _expanded = is_expanded;
    }

    void doRender(typename U::RendererImpl &render) const noexcept override {
        render.beginBlock();
        render.decoration(_expanded ? Decoration::ShortArrowDown : Decoration::ShortArrowRight);
        render.endBlock();

        render.decoration(Decoration::Space);

        render.value(this->label());
    }

    Request onClick() noexcept override {
        _expanded = not _expanded;

        this->background(_expanded ? expanded_background_color : folded_background_color);

        return Request::Rebuild;
    }

private:
    WidgetsView _widgets;
    bool _expanded{false};
};

}// namespace ui::widget