// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/algorithm.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/core/type_traits.hpp"
#include "kf/core/utility.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/ArrayList.hpp"
#include "kf/memory/Queue.hpp"
#include "kf/pattern/Singleton.hpp"
#include "kf/ui/Event.hpp"


namespace kf {

/// @brief User interface framework with widget-based rendering
/// @tparam R Renderer implementation type (must inherit from kf::ui::Render)
/// @note Singleton pattern ensures single UI instance with event queue and page management
template<typename R> struct UI final : Singleton<UI<R>> {
    friend Singleton<UI<R>>;

    using RenderImpl = R;   ///< Renderer implementation type
    using Event = ui::Event;///< UI event type alias

    struct Page;

    /// @brief Base widget class for all UI components
    /// @note All interactive UI elements inherit from this class
    struct Widget {
        /// @brief Construct widget and add to specified page
        /// @param root Page to add widget to
        explicit Widget(Page &root) {
            root.addWidget(*this);
        }

        /// @brief Default constructor (widget not attached to any page)
        explicit Widget() = default;

        /// @brief Render widget content (must be implemented by derived classes)
        /// @param render Renderer instance to use for drawing
        virtual void doRender(RenderImpl &render) const = 0;

        /// @brief Handle click event
        /// @return true if redraw required, false otherwise
        virtual bool onClick() { return false; }

        /// @brief Handle value change event
        /// @param direction Change direction (positive/negative)
        /// @return true if redraw required, false otherwise
        virtual bool onChange(int direction) { return false; }

        /// @brief External widget rendering with focus handling
        /// @param render Renderer instance to use for drawing
        /// @param focused true if widget currently has focus
        void render(RenderImpl &render, bool focused) const {
            if (focused) {
                render.beginContrast();
                doRender(render);
                render.endContrast();
            } else {
                doRender(render);
            }
        }
    };

    /// @brief UI page containing widgets and title
    struct Page {
    private:
        /// @brief Special widget for creating page navigation buttons
        /// @note Internal use only - use Page::link() for page navigation
        struct PageSetter final : Widget {
        private:
            Page &target;///< Target page for navigation

        public:
            /// @brief Construct page navigation widget
            /// @param target Page to navigate to when clicked
            explicit PageSetter(Page &target) :
                target{target} {}

            /// @brief Set target page as active on click
            /// @return true (redraw always required after page change)
            bool onClick() override {
                UI::instance().bindPage(target);
                return true;
            }

            /// @brief Render page navigation indicator
            /// @param render Renderer instance
            void doRender(RenderImpl &render) const override {
                render.arrow();
                render.string(target.title);
            }
        };

        ArrayList<Widget *> widgets{};///< List of widgets on this page
        const char *title;            ///< Page title displayed in header
        usize cursor{0};              ///< Current widget cursor position (focused widget index)
        PageSetter to_this{*this};    ///< Navigation widget to this page

    public:
        /// @brief Construct page with title
        /// @param title Page title string
        explicit Page(const char *title) :
            title{title} {}

        /// @brief Page behavior on entry
        virtual void onEntry() {}

        /// @brief Page behavior on leave
        virtual void onExit() {}

        /// @brief Page behavior on external update
        virtual void onUpdate() {}

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
        /// @param render Renderer instance to use for drawing
        /// @note Handles cursor positioning and widget focus
        void render(RenderImpl &render) {
            render.title(title);

            const auto available = render.widgetsAvailable();
            const auto start = (totalWidgets() > available) ? min(cursor, totalWidgets() - available) : 0;
            const auto end = min(start + available, totalWidgets());

            for (auto i = start; i < end; i += 1) {
                render.beginWidget(i);
                widgets[i]->render(render, i == cursor);
                render.endWidget();
            }
        }

        /// @brief Process incoming UI event
        /// @param event Event to process
        /// @return true if redraw required after event processing
        bool onEvent(Event event) {
            switch (event.type()) {
                case Event::Type::None: {
                    return false;
                }
                case Event::Type::Update: {
                    return true;
                }
                case Event::Type::PageCursorMove: {
                    return moveCursor(event.value());
                }
                case Event::Type::WidgetClick: {
                    if (totalWidgets() > 0) {
                        return widgets[cursor]->onClick();
                    }
                }
                case Event::Type::WidgetValueChange: {
                    if (totalWidgets() > 0) {
                        return widgets[cursor]->onChange(event.value());
                    }
                }
            }
            return false;
        }

        /// @brief Get total widget count on page
        /// @return Number of widgets on this page
        kf_nodiscard inline usize totalWidgets() const { return static_cast<int>(widgets.size()); }

    private:
        /// @brief Get maximum cursor position (last widget index)
        /// @return Maximum cursor index (totalWidgets() - 1)
        kf_nodiscard inline usize cursorPositionMax() const { return totalWidgets() - 1; }

        /// @brief Move cursor within page bounds
        /// @param delta Cursor movement delta (positive/negative)
        /// @return true if cursor position changed (redraw required)
        kf_nodiscard bool moveCursor(isize delta) {
            const auto last_cursor = cursor;
            cursor += delta;
            cursor = max(static_cast<isize>(cursor), 0);
            cursor = min(cursor, cursorPositionMax());
            return last_cursor != cursor;
        }
    };

private:
    Queue<Event> events{};     ///< Event queue for pending UI events
    Page *active_page{nullptr};///< Currently active page for rendering
    RenderImpl render_system{};///< Renderer implementation instance

public:
    /// @brief Access renderer configuration settings
    /// @return Reference to renderer settings structure
    typename RenderImpl::Settings &renderSettings() {
        return render_system.settings;
    }

    /// @brief Set active page for display
    /// @param page Page to make active (must remain valid)
    void bindPage(Page &page) {
        if (nullptr != active_page) {
            active_page->onExit();
        }

        active_page = &page;
        active_page->onEntry();
    }

    /// @brief Add event to processing queue
    /// @param event Event to queue for processing
    void addEvent(Event event) {
        events.push(event);
    }

    /// @brief Process active page update, pending events and render if needed
    /// @note Must be called regularly (e.g., in main loop)
    void poll() {
        if (nullptr == active_page) {
            return;
        }

        active_page->onUpdate();

        if (events.empty()) {
            return;
        }

        const bool render_required = active_page->onEvent(events.front());
        events.pop();

        if (not render_required) {
            return;
        }

        render_system.prepare();
        active_page->render(render_system);
        render_system.finish();
    }

    // Built-in widget implementations

    /// @brief Button widget for triggering actions on click
    struct Button final : Widget {
        using ClickHandler = Function<void()>;///< Button click handler type

    private:
        const char *label;    ///< Button label text
        ClickHandler on_click;///< Click event handler

    public:
        /// @brief Construct button with label and click handler
        /// @param root Page to add button to
        /// @param label Button display text
        /// @param on_click Function called when button is clicked
        explicit Button(
            Page &root,
            const char *label,
            ClickHandler on_click
        ) :
            Widget{root},
            label{label},
            on_click{kf::move(on_click)} {}

        /// @brief Handle button click event
        /// @return false (button click typically doesn't require redraw)
        bool onClick() override {
            if (nullptr != on_click) {
                on_click();
            }

            return false;
        }

        /// @brief Render button with block styling
        /// @param render Renderer instance
        void doRender(RenderImpl &render) const override {
            render.beginBlock();
            render.string(label);
            render.endBlock();
        }
    };

    /// @brief Checkbox widget for boolean input
    struct CheckBox final : Widget {
        using ChangeHandler = Function<void(bool)>;///< Checkbox state change handler

    private:
        ChangeHandler on_change;///< State change callback
        bool state;             ///< Current checkbox state

    public:
        /// @brief Construct checkbox with change handler (not attached to page)
        /// @param change_handler Function called when checkbox state changes
        /// @param default_state Initial checkbox state
        explicit CheckBox(
            ChangeHandler change_handler,
            bool default_state = false
        ) :
            on_change{kf::move(change_handler)},
            state{default_state} {}

        /// @brief Construct checkbox with change handler and add to page
        /// @param root Page to add checkbox to
        /// @param change_handler Function called when checkbox state changes
        /// @param default_state Initial checkbox state
        explicit CheckBox(
            Page &root,
            ChangeHandler change_handler,
            bool default_state = false
        ) :
            Widget{root},
            on_change{kf::move(change_handler)},
            state{default_state} {}

        /// @brief Toggle state on click
        /// @return true (redraw required after state change)
        bool onClick() override {
            setState(not state);
            return true;
        }

        /// @brief Set state based on direction
        /// @param direction Positive sets true, negative sets false
        /// @return true (redraw required after state change)
        bool onChange(int direction) override {
            setState(direction > 0);
            return true;
        }

        /// @brief Render checkbox with visual state indicator
        /// @param render Renderer instance
        void doRender(RenderImpl &render) const override {
            render.string(state ? "[ 1 ]==" : "--[ 0 ]");
        }

    private:
        /// @brief Update checkbox state and notify handler
        /// @param new_state New checkbox state
        void setState(bool new_state) {
            state = new_state;

            if (nullptr != on_change) {
                on_change(state);
            }
        }
    };

    /// @brief Combo box for selecting from predefined options
    /// @tparam T Value type for options
    /// @tparam N Number of options (must be ≥ 1)
    template<typename T, usize N> struct ComboBox final : Widget {
        static_assert(N >= 1, "N >= 1");

        using Value = T;///< Option value type

        /// @brief Combo box option item
        struct Item {
            const char *key;///< Display name for option
            T value;        ///< Value associated with option
        };

        using Container = Array<Item, N>;///< Container type for options

    private:
        const Container items;///< Available options
        T &value;             ///< Reference to current selected value (external storage)
        int cursor{0};        ///< Current selection index

    public:
        /// @brief Construct combo box (not attached to page)
        /// @param val Reference to variable storing current value
        /// @param items Array of option items
        explicit ComboBox(
            T &val,
            Container items
        ) :
            items{move(items)},
            value{val} {}

        /// @brief Construct combo box and add to page
        /// @param root Page to add combo box to
        /// @param val Reference to variable storing current value
        /// @param items Array of option items
        explicit ComboBox(
            Page &root,
            T &val,
            Container items
        ) :
            Widget{root},
            items{move(items)},
            value{val} {}

        /// @brief Change selection based on direction
        /// @param direction Navigation direction (positive/negative)
        /// @return true (redraw required after selection change)
        bool onChange(int direction) override {
            moveCursor(direction);
            value = items[cursor].value;
            return true;
        }

        /// @brief Render current selection
        /// @param render Renderer instance
        void doRender(RenderImpl &render) const override {
            render.variableBegin();
            render.string(items[cursor].key);
            render.variableEnd();
        }

    private:
        /// @brief Move selection cursor with circular wrapping
        /// @param delta Cursor movement delta
        void moveCursor(int delta) {
            cursor += delta;
            cursor += N;
            cursor %= N;
        }
    };

    /// @brief Display widget for showing read-only values
    /// @tparam T Type of value to display
    template<typename T> struct Display final : Widget {
    private:
        const T &value;///< Reference to value to display

    public:
        /// @brief Construct display widget and add to page
        /// @param root Page to add display to
        /// @param val Value to display (read-only reference)
        explicit Display(
            Page &root,
            const T &val
        ) :
            Widget{root},
            value{val} {}

        /// @brief Construct display widget (not attached to page)
        /// @param val Value to display (read-only reference)
        explicit Display(
            const T &val
        ) :
            value{val} {}

        /// @brief Render value with appropriate formatting
        /// @param render Renderer instance
        void doRender(RenderImpl &render) const override {
            kf_if_constexpr (kf::is_floating_point<T>::value) {
                render.number(static_cast<float>(value), 3);
            } else {
                render.number(value);
            }
        }
    };

    /// @brief Widget wrapper adding label to another widget
    /// @tparam W Type of widget being labeled (must inherit from Widget)
    template<typename W> struct Labeled final : Widget {
        static_assert(kf::is_base_of<Widget, W>::value, "W must be a Widget Subclass");

        using Impl = W;///< Type of wrapped widget implementation

    private:
        const char *label;///< Label text
        W impl;           ///< Wrapped widget instance

    public:
        /// @brief Construct labeled widget and add to page
        /// @param root Page to add labeled widget to
        /// @param label Text label for widget
        /// @param impl Widget to wrap with label
        explicit Labeled(
            Page &root,
            const char *label,
            W impl
        ) :
            Widget{root},
            label{label},
            impl{move(impl)} {}

        /// @brief Forward click event to wrapped widget
        /// @return Result from wrapped widget's onClick()
        bool onClick() override { return impl.onClick(); }

        /// @brief Forward change event to wrapped widget
        /// @param direction Change direction
        /// @return Result from wrapped widget's onChange()
        bool onChange(int direction) override { return impl.onChange(direction); }

        /// @brief Render label followed by wrapped widget
        /// @param render Renderer instance
        void doRender(RenderImpl &render) const override {
            render.string(label);
            render.colon();
            impl.doRender(render);
        }
    };

    /// @brief Spin box for adjusting numeric values with different modes
    /// @tparam T Numeric type for spin box value (must be arithmetic)
    template<typename T> struct SpinBox final : Widget {
        static_assert(kf::is_arithmetic<T>::value, "T must be arithmetic");

        using Value = T;///< Numeric value type

        /// @brief Spin box adjustment modes
        enum class Mode : unsigned char {
            Arithmetic,            ///< Add/subtract step value
            ArithmeticPositiveOnly,///< Add/subtract step, clamp at zero
            Geometric              ///< Multiply/divide by step value
        };

    private:
        bool is_step_setting_mode{false};///< true when adjusting step size, false when adjusting value
        const Mode mode;                 ///< Current adjustment mode
        T &value;                        ///< Reference to value being controlled
        T step;                          ///< Current step size

    public:
        /// @brief Construct spin box (not attached to page)
        /// @param value Reference to variable to control
        /// @param step Initial step size
        /// @param mode Adjustment mode (default: Arithmetic)
        explicit SpinBox(
            T &value,
            T step = static_cast<T>(1),
            Mode mode = Mode::Arithmetic
        ) :
            mode{mode},
            value{value},
            step{step} {}

        /// @brief Construct spin box and add to page
        /// @param root Page to add spin box to
        /// @param value Reference to variable to control
        /// @param step Initial step size
        /// @param mode Adjustment mode (default: Arithmetic)
        explicit SpinBox(
            Page &root,
            T &value,
            T step = static_cast<T>(1),
            Mode mode = Mode::Arithmetic
        ) :
            Widget{root},
            mode{mode},
            value{value},
            step{step} {}

        /// @brief Toggle between value adjustment and step adjustment modes
        /// @return true (redraw required after mode change)
        bool onClick() override {
            is_step_setting_mode = !is_step_setting_mode;
            return true;
        }

        /// @brief Adjust value or step based on current mode
        /// @param direction Adjustment direction (positive/negative)
        /// @return true (redraw required after adjustment)
        bool onChange(int direction) override {
            if (is_step_setting_mode) {
                changeStep(direction);
            } else {
                changeValue(direction);
            }
            return true;
        }

        /// @brief Render current value or step size based on mode
        /// @param render Renderer instance
        void doRender(RenderImpl &render) const override {
            render.variableBegin();

            if (is_step_setting_mode) {
                render.arrow();
                displayNumber(render, step);
            } else {
                displayNumber(render, value);
            }

            render.variableEnd();
        }

    private:
        /// @brief Display number with appropriate formatting
        /// @param render Renderer instance
        /// @param number Value to display
        void displayNumber(RenderImpl &render, const T &number) const {
            kf_if_constexpr (kf::is_floating_point<T>::value) {
                render.number(static_cast<float>(number), 4);
            } else {
                render.number(number);
            }
        }

        /// @brief Adjust controlled value based on mode and direction
        /// @param direction Adjustment direction (positive/negative)
        void changeValue(int direction) {
            if (mode == Mode::Geometric) {
                if (direction > 0) {
                    value *= step;
                } else {
                    value /= step;
                }
            } else {
                value += direction * step;

                if (mode == Mode::ArithmeticPositiveOnly and value < 0) {
                    value = 0;
                }
            }
        }

        /// @brief Adjust step size with multiplier protection
        /// @param direction Adjustment direction (positive/negative)
        void changeStep(int direction) {
            constexpr T step_multiplier{static_cast<T>(10)};

            if (direction > 0) {
                step *= step_multiplier;
            } else {
                step /= step_multiplier;

                kf_if_constexpr (kf::is_integral<T>::value) {
                    if (step < 1) { step = 1; }
                }
            }
        }
    };
};

}// namespace kf