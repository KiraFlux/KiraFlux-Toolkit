// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

// Std
#include <type_traits>
#include <utility>

// Lib
#include "kf/attributes.hpp"
#include "kf/Function.hpp"
#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/ArrayList.hpp"
#include "kf/memory/Queue.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/pattern/Singleton.hpp"

// Public UI API
#include "kf/ui/Event.hpp"
#include "kf/ui/StepMode.hpp"

// Private UI details
#include "kf/ui/detail/ComboBoxItem.hpp"
#include "kf/ui/detail/StepAdjuster.hpp"
#include "kf/ui/detail/ValueAdjuster.hpp"

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

    using StepMode = ui::StepMode;

    struct Page;// forward declaration for Widget

    /// @brief Base widget class for all UI components
    /// @note All interactive UI elements inherit from this class
    struct Widget {
        /// @brief Construct widget and add to specified page
        /// @param root Page to add widget to
        explicit Widget(Page &root) {
            root.addWidget(*this);
        }

        /// @brief Default constructor (widget not attached to any page)
        explicit Widget() noexcept = default;

        /// @brief Render widget content (must be implemented by derived classes)
        virtual void doRender(RenderImpl &render) const noexcept = 0;

        /// @brief Handle click event
        /// @return true if redraw required, false otherwise
        [[nodiscard]] virtual bool onClick() noexcept { return false; }

        /// @brief Handle value event
        /// @return true if redraw required, false otherwise
        [[nodiscard]] virtual bool onValue(EventValue value) noexcept { return false; }

        /// @brief External widget rendering with focus handling
        void render(RenderImpl &render, bool focused) const noexcept {
            if (focused) {
                render.beginFocused();
                doRender(render);
                render.endFocused();
            } else {
                doRender(render);
            }
        }
    };

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

        /// @brief Page behavior on external update
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
        [[nodiscard]] inline usize widgetsTotal() const noexcept { return widgets.size(); }

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

private:
    Queue<Event> events{};     ///< Event queue for pending UI events
    Page *active_page{nullptr};///< Currently active page for rendering
    RenderImpl render_system{};///< Renderer implementation instance

public:
    /// @brief Access renderer configuration settings
    /// @return Reference to renderer settings structure
    [[nodiscard]] RenderConfig &renderConfig() noexcept { return render_system.config; }

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

    // Helpful components

    template<typename T> struct HasChangeHandler {
        Function<void(T)> change_handler{nullptr};

    protected:
        void invokeHandler(T value) const noexcept {
            if (change_handler) {
                change_handler(value);
            }
        }
    };

    // Built-in widget implementations

    /// @brief Button widget for triggering actions on click
    struct Button final : Widget {
        using ClickHandler = Function<void()>;///< Button click handler type

    private:
        StringView label;///< Button label text

    public:
        ClickHandler on_click{nullptr};///< Click event handler

        explicit Button(Page &root, StringView label) :
            Widget{root}, label{label} {}

        /// @brief Handle button click event
        [[nodiscard]] bool onClick() noexcept override {
            if (on_click) {
                on_click();
            }

            return false;// button click typically doesn't require redraw
        }

        /// @brief Render button with block styling
        void doRender(RenderImpl &render) const noexcept override {
            render.beginBlock();
            render.value(label);
            render.endBlock();
        }
    };

    /// @brief Checkbox widget for boolean input
    struct CheckBox final : Widget, HasChangeHandler<bool> {
    private:
        bool state_;

    public:
        explicit CheckBox(bool default_state = false) noexcept :
            state_{default_state} {}

        explicit CheckBox(Page &root, bool default_state = false) :
            Widget{root}, state_{default_state} {}

        void setState(bool state) noexcept {
            state_ = state;
            HasChangeHandler<bool>::invokeHandler(state_);
        }

        [[nodiscard]] bool state() const noexcept {
            return state_;
        }

        [[nodiscard]] bool onClick() noexcept override {
            setState(not state_);
            return true;
        }

        [[nodiscard]] bool onValue(EventValue value) noexcept override {
            setState(value > 0);
            return true;
        }

        /// @brief Render checkbox with visual state indicator
        void doRender(RenderImpl &render) const noexcept override {
            render.checkbox(state_);
        }
    };

    /// @brief Combo box for selecting from predefined options
    /// @tparam T Value type for options
    /// @tparam N Number of options (must be >= 1)
    template<typename T, usize N> struct ComboBox final : Widget, HasChangeHandler<T> {
        static_assert(N >= 1, "N >= 1");

        using Value = T;                             ///< ComboBox value type
        using Item = kf::ui::detail::ComboBoxItem<T>;///< Item type (in option)
        using ItemContainer = Array<Item, N>;        ///< Container type for options

    private:
        const ItemContainer items;///< Available options
        usize cursor{0};          ///< Current selection index

    public:
        explicit ComboBox(ItemContainer items) noexcept :
            items{items} {}

        explicit ComboBox(Page &root, ItemContainer items) :
            Widget{root}, items{items} {}

        /// @brief Change selection based on direction
        /// @param value Navigation direction (positive/negative)
        [[nodiscard]] bool onValue(EventValue value) noexcept override {
            moveCursor(value);
            HasChangeHandler<T>::invokeHandler(items[cursor].value());
            return true;// redraw required after selection change
        }

        /// @brief Render current selection
        void doRender(RenderImpl &render) const noexcept override {
            render.beginAltBlock();
            render.value(items[cursor].key());
            render.endAltBlock();
        }

    private:
        /// @brief Move selection cursor with circular wrapping
        void moveCursor(isize delta) noexcept {
            cursor = (cursor + delta + N) % N;
        }
    };

    /// @brief Display widget for showing read-only values
    /// @tparam T Type of value to display
    template<typename T> struct Display final : Widget {
    private:
        const T &value;///< Reference to value to display

    public:
        explicit Display(Page &root, const T &val) :
            Widget{root}, value{val} {}

        explicit Display(const T &val) noexcept :
            value{val} {}

        /// @brief Render value with appropriate formatting
        void doRender(RenderImpl &render) const noexcept override {
            render.value(value);
        }
    };

    /// @brief Widget wrapper adding label to another widget
    /// @tparam W Type of widget being labeled (must inherit from Widget)
    template<typename W> struct Labeled final : Widget {
        static_assert(std::is_base_of<Widget, W>::value, "W must be a Widget Subclass");

        using Impl = W;///< Type of wrapped widget implementation

    private:
        StringView label;///< Label text

    public:
        W impl;///< Wrapped widget instance

        explicit Labeled(Page &root, StringView label, W impl) :
            Widget{root}, label{label}, impl{std::move(impl)} {}

        /// @brief Forward click event to wrapped widget
        /// @return Result from wrapped widget's onClick()
        [[nodiscard]] bool onClick() noexcept override { return impl.onClick(); }

        /// @brief Forward change event to wrapped widget
        /// @return Result from wrapped widget's onValue()
        [[nodiscard]] bool onValue(EventValue value) noexcept override { return impl.onValue(value); }

        /// @brief Render label followed by wrapped widget
        void doRender(RenderImpl &render) const noexcept override {
            render.value(label);
            render.colon();
            impl.doRender(render);
        }
    };

    /// @brief Spin box for adjusting numeric values with different modes
    /// @tparam T Numeric type for spin box value (must be arithmetic)
    template<typename T, StepMode M> struct SpinBox final : Widget, HasChangeHandler<T> {
        using Value = T;                   ///< Numeric value type
        static constexpr auto step_mode{M};///< Specialization step mode

    private:
        using StepAdjuster = kf::ui::detail::StepAdjuster<T>;
        using ValueAdjuster = kf::ui::detail::ValueAdjuster<T, M>;

        T value_;                        ///< Reference to value being controlled
        T step;                          ///< Current step size
        bool is_step_setting_mode{false};///< true when adjusting step size, false when adjusting value

    public:
        explicit SpinBox(
            T default_value = T{},
            T step = StepAdjuster::default_step) noexcept :
            value_{default_value}, step{step} {}

        explicit SpinBox(
            Page &root,
            T default_value = T{},
            T step = StepAdjuster::default_step) :
            Widget{root}, value_{default_value}, step{step} {}

        void setValue(T value) noexcept {
            value_ = value;
            HasChangeHandler<T>::invokeHandler(value_);
        }

        [[nodiscard]] T value() const noexcept { return value_; }

        /// @brief Toggle between value adjustment and step adjustment modes
        /// @return true (redraw required after mode change)
        [[nodiscard]] bool onClick() noexcept override {
            is_step_setting_mode = not is_step_setting_mode;
            return true;
        }

        /// @brief Adjust value or step based on current mode
        /// @param direction Adjustment direction (positive/negative)
        [[nodiscard]] bool onValue(EventValue direction) noexcept override {
            if (is_step_setting_mode) {
                StepAdjuster::adjust(step, direction);
            } else {
                ValueAdjuster::adjust(value_, step, direction);
                HasChangeHandler<T>::invokeHandler(value_);
            }
            return true;// redraw required after adjustment
        }

        /// @brief Render current value or step size based on mode
        void doRender(RenderImpl &render) const noexcept override {
            render.beginAltBlock();

            if (is_step_setting_mode) {
                render.arrow();
                render.value(step);
            } else {
                render.value(value_);
            }

            render.endAltBlock();
        }
    };
};

}// namespace kf