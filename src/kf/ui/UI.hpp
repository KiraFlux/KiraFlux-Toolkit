// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Queue.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Singleton.hpp"
#include "kf/mixin/TimedPollable.hpp"

#include "kf/ui/internal/UiTraits.hpp"
#include "kf/ui/render/Render.hpp"
#include "kf/ui/widgets/Button.hpp"
#include "kf/ui/widgets/CheckBox.hpp"
#include "kf/ui/widgets/ComboBox.hpp"
#include "kf/ui/widgets/Display.hpp"
#include "kf/ui/widgets/Labeled.hpp"
#include "kf/ui/widgets/Slider.hpp"
#include "kf/ui/widgets/SpinBox.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui {

/// @brief User interface framework with widget-based rendering
/// @tparam R Render impl
/// @tparam E Event impl
/// @note Singleton pattern ensures single UI instance with event queue and page management
template<typename R, typename E> struct UI final : kf::mixin::Singleton<UI<R, E>>, kf::mixin::TimedPollable<UI<R, E>> {
    kf_crtp_check(R, kf::ui::render::RenderTag);

    using RenderImpl = R;                            ///< Renderer implementation type
    using RenderConfig = typename RenderImpl::Config;///< Renderer Configuration type

    using Event = E;                         ///< UI Event type
    using EventValue = typename Event::Value;///< UI Event Value type

    using StepMode = internal::StepMode;
    using ValuePlacement = internal::ValuePlacement;

    struct Page;

private:
    using InternalUI = internal::UI<R, EventValue, Page>;

public:
    /// @brief Base widget type (provided for inheritance or generic references)
    using Widget = widgets::Widget<Traits>;

    /// @brief Button widget with click handler
    /// @note Usage: `Button(page, label)`
    using Button = widgets::Button<Traits>;

    /// @brief Checkbox with boolean state and change handler
    using CheckBox = widgets::CheckBox<Traits>;

    /// @brief Combo box for selecting from a fixed set of options
    /// @tparam T Value type of options
    template<typename T> using ComboBox = widgets::ComboBox<Traits, T>;

    /// @brief Read-only display of a value (by reference)
    /// @tparam T Type of displayed value (must outlive the widget)
    template<typename T> using Display = widgets::Display<Traits, T>;

    /// @brief Wrapper that adds a label to another widget
    using Labeled = widgets::Labeled<Traits>;

    /// @brief Spin box for numeric adjustment with configurable step mode
    /// @tparam T Arithmetic type (int, float, etc.)
    /// @tparam M Step mode (Arithmetic, ArithmeticPositiveOnly, Geometric)
    template<typename T, typename A> using SpinBox = widgets::SpinBox<Traits, T, A>;

    /// @brief Slider for numeric adjustment with constraints
    /// @tparam T Arithmetic type (int, float, etc.)
    template<typename T> using Slider = widgets::Slider<Traits, T>;

    /// @brief Access renderer configuration
    /// @return Reference to renderer config structure
    [[nodiscard]] RenderConfig &renderConfig() noexcept { return _render_config; }

    /// @brief Set active page for display
    /// @param page Page to make active (must remain valid)
    void bindPage(Page &page) noexcept {
        if (nullptr != _active_page) {
            _active_page->onExit();
        }

        _active_page = &page;
        _active_page->onEntry();
    }

    /// @brief Add event to processing queue
    void addEvent(Event event) {
        _events.push(event);
    }

private:
    /// @brief Special widget for creating page navigation buttons
    /// @note Internal use only - use Page::link() for page navigation
    struct PageSetter final : Widget {

        explicit PageSetter(Page &target) noexcept :
            _target{target} {}

        /// @brief Set target page as active on click
        [[nodiscard]] bool onClick() noexcept override {
            UI::instance().bindPage(_target);
            return true;// redraw always required after page change
        }

        void doRender(RenderImpl &render) const noexcept override {
            render.arrow();
            render.value(_target.label());
        }

    private:
        Page &_target;///< Target page for navigation
    };

public:
    /// @brief UI page containing widgets and label
    struct Page : mixin::NonCopyable, mixin::Labeled {

        using mixin::Labeled::Labeled;

        /// @brief Page behavior on entry
        virtual void onEntry() noexcept {}

        /// @brief Page behavior on leave
        virtual void onExit() noexcept {}

        /// @brief Page behavior on UI polling
        virtual void onUpdate(math::Milliseconds now) noexcept {}

        /// @brief Get 'go to this page' Widget
        Widget &link() noexcept { return _to_this; }

        /// @brief Render page content to display
        /// @note Handles cursor positioning and widget focus
        void render(RenderImpl &render) noexcept {
            render.title(this->label());

            if (nullptr == _widgets.data()) { return; }

            const usize available = render.widgetsAvailable();
            const usize start = (_widgets.size() > available) ? kf::min(static_cast<usize>(_cursor), _widgets.size() - available) : 0;
            const usize end = kf::min(start + available, _widgets.size());

            for (auto i = start; i < end; i += 1) {
                auto widget = _widgets[i];

                if (nullptr == widget) { continue; }

                render.beginWidget(i);

                const bool is_selected = (i == _cursor);
                widget->render(render, is_selected);

                render.endWidget();
            }
        }

        /// @brief Process incoming UI event
        /// @return true if redraw required after event processing
        [[nodiscard]] bool onEvent(Event event) noexcept {
            switch (event.kind()) {
                case Event::Kind::Update: {
                    return true;
                }
                case Event::Kind::PageCursorMove: {
                    return moveCursor(event.value());
                }
                case Event::Kind::WidgetClick: {
                    if (_widgets.size() > 0) {
                        return _widgets[_cursor]->onClick();
                    }
                }
                case Event::Kind::WidgetValueChange: {
                    if (_widgets.size() > 0) {
                        return _widgets[_cursor]->onEventValue(event.value());
                    }
                }
            }
            return false;
        }

        /// @brief Get widgets on page
        [[nodiscard]] memory::Slice<Widget *> widgets() noexcept { return _widgets; }

    protected:
        /// @brief Set widgets on page
        void widgets(memory::Slice<Widget *> new_widgets) noexcept { _widgets = new_widgets; }

    private:
        memory::Slice<Widget *> _widgets{};///< Widgets on this page
        PageSetter _to_this{*this};        ///< Navigation widget to this page
        isize _cursor{0};                  ///< Current widget cursor position (focused widget index)

        /// @brief Move cursor within page bounds
        /// @param delta Cursor movement delta (positive/negative)
        /// @return true if cursor position changed (redraw required)
        [[nodiscard]] bool moveCursor(isize delta) noexcept {
            const auto n = _widgets.size();
            if (n > 1) {
                _cursor = (_cursor + delta + n) % n;
                return true;
            } else {
                return false;
            }
        }
    };

private:
    memory::Queue<Event> _events{};           ///< Event queue for pending UI events
    RenderConfig _render_config{};            ///< Render Configuration
    RenderImpl _render_system{_render_config};///< Renderer system implementation instance
    Page *_active_page{nullptr};              ///< Currently active page for rendering

    // impl
    using This = UI<R, E>;

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(math::Milliseconds now) noexcept {
        // Process active page update, pending events and render if needed

        if (nullptr == _active_page) { return; }

        _active_page->onUpdate(now);

        if (_events.empty()) { return; }

        constexpr usize max_events_per_poll{20};
        usize events_processed{0};

        bool render_required{false};

        while (not _events.empty() and events_processed < max_events_per_poll) {
            render_required |= _active_page->onEvent(_events.front());
            events_processed += 1;
            _events.pop();
        }

        if (render_required) {
            _render_system.prepare();
            _active_page->render(_render_system);
            _render_system.finish();
        }
    }
};

}// namespace kf::ui