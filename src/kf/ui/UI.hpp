// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/ArrayList.hpp"
#include "kf/memory/Queue.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/pattern/Singleton.hpp"
#include "kf/ui/internal/UI.hpp"

namespace kf {

/// @brief User interface framework with widget-based rendering
/// @tparam R Renderer implementation type (must inherit from kf::ui::Render)
/// @note Singleton pattern ensures single UI instance with event queue and page management
template<typename R, typename E> struct UI final : Singleton<UI<R, E>> {
    friend struct Singleton<UI<R, E>>;

    using RenderImpl = R;                            ///< Renderer implementation type
    using RenderConfig = typename RenderImpl::Config;///< Renderer Configuration type

    using Event = E;                         ///< UI Event type
    using EventValue = typename Event::Value;///< UI Event Value type

    using StepMode = kf::ui::internal::StepMode;

    struct Page;

private:
    using InternalUI = kf::ui::internal::UI<R, EventValue, Page>;

public:
    /// @brief Base widget type (provided for inheritance or generic references)
    using Widget = typename InternalUI::Widget;

    /// @brief Button widget with click handler
    /// @note Usage: `Button(page, label)`
    using Button = typename InternalUI::Button;

    /// @brief Checkbox with boolean state and change handler
    using CheckBox = typename InternalUI::CheckBox;

    /// @brief Combo box for selecting from a fixed set of options
    /// @tparam T Value type of options
    /// @tparam N Number of options (compile-time)
    template<typename T, usize N> using ComboBox = typename InternalUI::template ComboBox<T, N>;

    /// @brief Read-only display of a value (by reference)
    /// @tparam T Type of displayed value (must outlive the widget)
    template<typename T> using Display = typename InternalUI::template Display<T>;

    /// @brief Wrapper that adds a label to another widget
    /// @tparam W Widget type being labeled (must inherit from Widget)
    template<typename W> using Labeled = typename InternalUI::template Labeled<W>;

    /// @brief Spin box for numeric adjustment with configurable step mode
    /// @tparam T Arithmetic type (int, float, etc.)
    /// @tparam M Step mode (Arithmetic, ArithmeticPositiveOnly, Geometric)
    template<typename T, StepMode M> using SpinBox = typename InternalUI::template SpinBox<T, M>;

private:
    Queue<Event> events{};     ///< Event queue for pending UI events
    Page *active_page{nullptr};///< Currently active page for rendering
    RenderImpl render_system{};///< Renderer implementation instance

public:
    /// @brief Access renderer configuration settings
    /// @return Reference to renderer settings structure
    [[nodiscard]] RenderConfig &renderConfig() noexcept { return render_system.getConfig(); }

    /// @brief Set active page for display
    /// @param page Page to make active (must remain valid)
    void bindPage(Page &page) noexcept {
        if (nullptr != active_page) {
            active_page->onExit();
        }

        active_page = &page;
        active_page->onEntry();
    }

    /// @brief Add event to processing queue
    void addEvent(Event event) {
        events.push(event);
    }

    /// @brief Process active page update, pending events and render if needed
    /// @note Must be called regularly (e.g., in main loop)
    void poll(Milliseconds now) noexcept {
        if (nullptr == active_page) { return; }

        active_page->onUpdate(now);

        if (events.empty()) { return; }

        constexpr usize max_events_per_poll{20};
        usize events_processed{0};

        bool render_required{false};

        while (not events.empty() and events_processed < max_events_per_poll) {
            render_required |= active_page->onEvent(events.front());
            events_processed += 1;
            events.pop();
        }

        if (render_required) {
            render_system.prepare();
            active_page->render(render_system);
            render_system.finish();
        }
    }

private:
    /// @brief Special widget for creating page navigation buttons
    /// @note Internal use only - use Page::link() for page navigation
    struct PageSetter final : Widget {
    private:
        Page &target;///< Target page for navigation

    public:
        explicit PageSetter(Page &target) noexcept :
            target{target} {}

        /// @brief Set target page as active on click
        [[nodiscard]] bool onClick() noexcept override {
            UI::instance().bindPage(target);
            return true;// redraw always required after page change
        }

        void doRender(RenderImpl &render) const noexcept override {
            render.arrow();
            render.value(target.title());
        }
    };

public:
    /// @brief UI page containing widgets and title
    struct Page {
    private:
        ArrayList<Widget *> widgets{};///< List of widgets on this page
        PageSetter to_this{*this};    ///< Navigation widget to this page
        usize cursor{0};              ///< Current widget cursor position (focused widget index)
        StringView title_;            ///< Page title displayed in header

    public:
        explicit Page(StringView title) :
            title_{title} {}

        /// @brief Page behavior on entry
        virtual void onEntry() noexcept {}

        /// @brief Page behavior on leave
        virtual void onExit() noexcept {}

        /// @brief Page behavior on UI polling
        virtual void onUpdate(Milliseconds now) noexcept {}

        /// @brief Add widget to this page
        /// @param widget Widget to add (must remain valid for page lifetime)
        void addWidget(Widget &widget) {
            widgets.push_back(&widget);
        }

        /// @brief Create bidirectional navigation link between pages
        /// @param other Page to link with (adds navigation widgets to both pages)
        void link(Page &other) {
            this->addWidget(other.to_this);
            other.addWidget(this->to_this);
        }

        /// @brief Render page content to display
        /// @note Handles cursor positioning and widget focus
        void render(RenderImpl &render) noexcept {
            render.title(title_);

            const auto available = render.widgetsAvailable();
            const auto start = (widgetsTotal() > available) ? kf::min(cursor, widgetsTotal() - available) : 0;
            const auto end = kf::min(start + available, widgetsTotal());

            for (auto i = start; i < end; i += 1) {
                render.beginWidget(i);
                widgets[i]->render(render, i == cursor);
                render.endWidget();
            }
        }

        /// @brief Process incoming UI event
        /// @return true if redraw required after event processing
        [[nodiscard]] bool onEvent(Event event) noexcept {
            switch (event.type()) {
                case Event::Type::Update: {
                    return true;
                }
                case Event::Type::PageCursorMove: {
                    return moveCursor(event.value());
                }
                case Event::Type::WidgetClick: {
                    if (widgetsTotal() > 0) {
                        return widgets[cursor]->onClick();
                    }
                }
                case Event::Type::WidgetValueChange: {
                    if (widgetsTotal() > 0) {
                        return widgets[cursor]->onValue(event.value());
                    }
                }
            }
            return false;
        }

        /// @brief Get total widget count on page
        [[nodiscard]] usize widgetsTotal() const noexcept { return widgets.size(); }

        /// @brief Get page title
        [[nodiscard]] StringView title() const noexcept { return title_; }

    private:
        /// @brief Move cursor within page bounds
        /// @param delta Cursor movement delta (positive/negative)
        /// @return true if cursor position changed (redraw required)
        [[nodiscard]] bool moveCursor(isize delta) noexcept {
            const auto n = widgetsTotal();
            if (n > 1) {
                cursor = (cursor + delta + n) % n;
                return true;
            } else {
                return false;
            }
        }
    };
};

}// namespace kf