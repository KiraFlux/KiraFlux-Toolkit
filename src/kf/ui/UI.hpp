// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/algorithm.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Queue.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/Labeled.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/TimedPollable.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Decoration.hpp"
#include "kf/ui/Layout.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"
#include "kf/ui/render/Renderer.hpp"
#include "kf/ui/widgets/Button.hpp"
#include "kf/ui/widgets/CheckBox.hpp"
#include "kf/ui/widgets/ComboBox.hpp"
#include "kf/ui/widgets/Display.hpp"
#include "kf/ui/widgets/Labeled.hpp"
#include "kf/ui/widgets/Slider.hpp"
#include "kf/ui/widgets/SpinBox.hpp"

namespace kf::ui {

/// @brief User interface framework with widget-based rendering
/// @tparam U UI Traits implementation
template<typename U> struct UI :

    mixin::NonCopyable,
    mixin::TimedPollable<UI<U>>

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    /// @brief UI Traits implementation
    struct Traits : U {};

    using Widget = typename U::Widget;

    /// @brief Button widget with click handler
    struct Button : widgets::Button<Traits> {
        using widgets::Button<Traits>::Button;
    };

    /// @brief Checkbox with boolean state and change handler
    struct CheckBox : widgets::CheckBox<Traits> {
        using widgets::CheckBox<Traits>::CheckBox;
    };

    /// @brief Combo box for selecting from a fixed set of options
    /// @tparam T Value type of options
    template<typename T> struct ComboBox : widgets::ComboBox<Traits, T> {
        using widgets::ComboBox<Traits, T>::ComboBox;
    };

    /// @brief Read-only display of a value (by reference)
    /// @tparam T Type of displayed value (must outlive the widget)
    template<typename T> struct Display : widgets::Display<Traits, T> {
        using widgets::Display<Traits, T>::Display;
    };

    /// @brief Wrapper that adds a label to another widget
    struct Labeled : widgets::Labeled<Traits> {
        using widgets::Labeled<Traits>::Labeled;
    };

    /// @brief Spin box for numeric adjustment with configurable step mode
    /// @tparam T Arithmetic type (int, float, etc.)
    /// @tparam A Adjuster (Arithmetic, ArithmeticPositiveOnly, Geometric)
    template<typename T, typename A> struct SpinBox : widgets::SpinBox<Traits, T, A> {
        using widgets::SpinBox<Traits, T, A>::SpinBox;
    };

    /// @brief Slider for numeric adjustment with constraints
    /// @tparam T Arithmetic type (int, float, etc.)
    template<typename T> struct Slider : widgets::Slider<Traits, T> {
        using widgets::Slider<Traits, T>::Slider;
    };

    struct Page;// forward declaration

    explicit constexpr UI(typename Traits::RendererImpl &renderer) noexcept : _renderer{renderer} {}

    /// @brief Set active page
    /// @param page Page to make active (must remain valid)
    void activePage(Page &page) noexcept {
        if (_active_page.isSome()) {
            _active_page.unwrap().onExit();
        }
        page.onEntry();
        _active_page = someRef(page);
    }

    /// @brief Requests a UI redraw on the next poll cycle
    void requestRender() noexcept {
        _renderer.requestRender();
    }

    /// @brief Get readonly access to active page
    constexpr auto activePage() const noexcept -> kf::Option<Page &> {
        return _active_page;
    }

    /// @brief Add event to processing queue
    void addEvent(typename Traits::EventImpl event) noexcept {
        _events.push(event);
    }

private:
    /// @brief Special widget for creating page navigation buttons
    /// @note Internal use only - use Page::link() for page navigation
    struct PageSetter : Widget {
        explicit PageSetter(Page &target) noexcept : _target{target}, Widget{Style::defaults()} {
            this->hint("Navigate to page..");
        }

        /// @brief Set target page as active on click
        [[nodiscard]] bool onClick() noexcept override {
            _target._ui.activePage(_target);
            return true;// redraw always required after page change
        }

        void doRender(typename Traits::RendererImpl &render) const noexcept override {
            render.decoration(Decoration::LongArrowRight);
            render.value(_target.label());
        }

    private:
        Page &_target;///< Target page for navigation
    };

public:
    /// @brief UI page containing widgets and label
    struct Page :

        mixin::NonCopyable,
        mixin::Labeled,
        mixin::TimedPollable<Page>

    {
        friend struct PageSetter;

        using WidgetsView = Slice<Widget *>;

        explicit constexpr Page(UI &ui, Layout layout = Layout::Vertical) noexcept :
            _ui{ui}, _layout{layout} {}

        /// @brief Build page content
        /// @note Not for direct call. Use `Page::requestBuild()`
        virtual WidgetsView build() noexcept { return {}; }

        /// @brief Page behavior on entry
        virtual void onEntry() noexcept {}

        /// @brief Page behavior on leave
        virtual void onExit() noexcept {}

    protected:
        /// @brief Page behavior on UI polling
        virtual void onPoll(math::Milliseconds now) noexcept {}

    public:
        /// @brief Request Page content build
        void requestBuild() noexcept {
            _build_requested = true;
        }

        /// @brief Get page layout
        [[nodiscard]] constexpr Layout layout() noexcept {
            return _layout;
        }

        /// @brief Get Mutable access to 'go to this page' Widget
        [[nodiscard]] constexpr Widget &link() noexcept {
            return _to_this;
        }

        /// @brief Get Readonly access to 'go to this page' Widget
        [[nodiscard]] constexpr const Widget &link() const noexcept {
            return _to_this;
        }

        /// @brief Get selected widget
        [[nodiscard]] constexpr auto selectedWidget() const noexcept -> Option<const Widget &> {
            return _widgets.empty() ? none : someRef<const Widget &>(*_widgets[_cursor]);
        }

        /// @brief Render page content to display.
        /// @param render Renderer instance.
        /// @note Handles cursor positioning and widget focus.
        void render(typename Traits::RendererImpl &render) noexcept {
            const usize available = render.beginPage(this->label(), _layout);

            if (_widgets.empty()) {
                // TODO: render placeholder page content
            } else {
                const usize start = (_widgets.size() > available) ? kf::min(static_cast<usize>(_cursor), _widgets.size() - available) : 0;
                const usize end = kf::min(start + available, _widgets.size());

                for (auto i = start; i < end; i += 1) {
                    auto widget = _widgets[i];

                    if (nullptr == widget) {
                        // TODO: render placeholder widget
                    } else {
                        widget->render(render, i, (i == _cursor));
                    }
                }
            }

            render.endPage();
        }

        /// @brief Add event to processing queue
        /// @param event The event to be processed
        void onEvent(typename Traits::EventImpl event) noexcept {
            using Kind = typename Traits::EventImpl::Kind;
            switch (event.kind()) {
                case Kind::PageCursorMove: {
                    moveCursor(event.value());
                    return;
                }
                case Kind::WidgetClick: {
                    if (not _widgets.empty()) {
                        if (_widgets[_cursor]->onClick()) {
                            _ui.requestRender();
                        }
                        return;
                    }
                }
                case Kind::WidgetValue: {
                    if (not _widgets.empty()) {
                        if (_widgets[_cursor]->onEventValue(event.value())) {
                            _ui.requestRender();
                        }
                        return;
                    }
                }
            }
        }

    protected:
        /// @brief Set page layout
        void layout(Layout new_layout) noexcept {
            _layout = new_layout;
        }

        /// @brief Set page cursor
        void cursor(usize new_cursor) noexcept {
            _cursor = clamp<isize>(new_cursor, 0, _widgets.size() - 1);
        }

        UI &_ui;

    private:
        PageSetter _to_this{*this};///< Navigation widget to this page
        WidgetsView _widgets{};    ///< Widgets on this page
        isize _cursor{0};          ///< Current widget cursor position (focused widget index)
        Layout _layout;            ///< Current layout
        bool _build_requested{true};

        /// @brief Move cursor within page bounds
        /// @param delta Cursor movement delta (positive/negative)
        void moveCursor(isize delta) noexcept {
            const auto n = _widgets.size();
            if (n > 1) {
                _cursor = (_cursor + delta + n) % n;
                _ui.requestRender();
            }
        }

        KF_IMPL_TIMED_POLLABLE(Page);
        void pollImpl(math::Milliseconds now) noexcept {
            onPoll(now);

            if (_build_requested) {
                _build_requested = false;

                auto new_widgets = build();

                if (new_widgets.size() == _widgets.size()) {
                    for (auto i = 0u; i < _widgets.size(); i += 1) {
                        if (new_widgets[i] != _widgets[i]) {
                            _ui.requestRender();
                            break;
                        }
                    }
                } else {
                    _ui.requestRender();
                }

                _widgets = new_widgets;
                cursor(_cursor);
            }
        }
    };

private:
    memory::Queue<typename Traits::EventImpl> _events{};///< Event queue for pending UI events
    typename Traits::RendererImpl &_renderer;           ///< Renderer system implementation
    Option<Page &> _active_page{none};                  ///< Currently active page for rendering

    using This = UI<U>;

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(math::Milliseconds now) noexcept {
        if (_active_page.isNone()) { return; }

        if (not _events.empty()) {
            constexpr usize max_events_per_poll{20};
            usize events_processed{0};

            while (not _events.empty() and events_processed < max_events_per_poll) {
                _active_page.unwrap().onEvent(_events.front());

                events_processed += 1;
                _events.pop();
            }
        }

        _active_page.unwrap().poll(now);

        if (_renderer.renderRequested()) {
            _renderer.beginFrame();
            _active_page.unwrap().render(_renderer);
            _renderer.endFrame();
        }
    }
};

}// namespace kf::ui