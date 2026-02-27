// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <utility>

#include "kf/Function.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/StringView.hpp"

#include "kf/ui/internal/ComboBoxItem.hpp"
#include "kf/ui/internal/StepAdjuster.hpp"
#include "kf/ui/internal/StepMode.hpp"
#include "kf/ui/internal/ValueAdjuster.hpp"

namespace kf::ui::internal {

/// @brief Internal UI Definitions
/// @tparam R Render System Implementation
/// @tparam Ev Event Value Type
/// @tparam P Page Type
template<typename R, typename Ev, typename P> struct UI final {

    /// @brief Base widget class for all UI components
    /// @note All interactive UI elements inherit from this class
    struct Widget {
        /// @brief Construct widget and add to specified page
        /// @param root Page to add widget to
        explicit Widget(P &root) {
            root.addWidget(*this);
        }

        /// @brief Default constructor (widget not attached to any page)
        explicit Widget() noexcept = default;

        /// @brief Render widget content (must be implemented by derived classes)
        virtual void doRender(R &render) const noexcept = 0;

        /// @brief Handle click event
        /// @return true if redraw required, false otherwise
        [[nodiscard]] virtual bool onClick() noexcept { return false; }

        /// @brief Handle value event
        /// @return true if redraw required, false otherwise
        [[nodiscard]] virtual bool onValue(Ev value) noexcept { return false; }

        /// @brief External widget rendering with focus handling
        void render(R &render, bool focused) const noexcept {
            if (focused) {
                render.beginFocused();
                doRender(render);
                render.endFocused();
            } else {
                doRender(render);
            }
        }
    };

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

        explicit Button(P &root, StringView label) :
            Widget{root}, label{label} {}

        /// @brief Handle button click event
        [[nodiscard]] bool onClick() noexcept override {
            if (on_click) {
                on_click();
            }

            return false;// button click typically doesn't require redraw
        }

        /// @brief Render button with block styling
        void doRender(R &render) const noexcept override {
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

        explicit CheckBox(P &root, bool default_state = false) :
            Widget{root}, state_{default_state} {}

        void setState(bool new_state) noexcept {
            if (new_state != state_) {
                state_ = new_state;
                HasChangeHandler<bool>::invokeHandler(state_);
            }
        }

        [[nodiscard]] bool state() const noexcept {
            return state_;
        }

        [[nodiscard]] bool onClick() noexcept override {
            setState(not state_);
            return true;
        }

        [[nodiscard]] bool onValue(Ev value) noexcept override {
            setState(value > 0);
            return true;
        }

        /// @brief Render checkbox with visual state indicator
        void doRender(R &render) const noexcept override {
            render.checkbox(state_);
        }
    };

    /// @brief Combo box for selecting from predefined options
    /// @tparam T Value type for options
    /// @tparam N Number of options (must be >= 1)
    template<typename T, usize N> struct ComboBox final : Widget, HasChangeHandler<T> {
        static_assert(N >= 1, "N >= 1");

        using Value = T;                     ///< ComboBox value type
        using Item = ComboBoxItem<T>;        ///< Item type (in option)
        using ItemContainer = Array<Item, N>;///< Container type for options

    private:
        const ItemContainer items;///< Available options
        usize cursor{0};          ///< Current selection index

    public:
        explicit ComboBox(ItemContainer items) noexcept :
            items{items} {}

        explicit ComboBox(P &root, ItemContainer items) :
            Widget{root}, items{items} {}

        /// @brief Change selection based on direction
        /// @param value Navigation direction (positive/negative)
        [[nodiscard]] bool onValue(Ev value) noexcept override {
            moveCursor(value);
            HasChangeHandler<T>::invokeHandler(items[cursor].value());
            return true;// redraw required after selection change
        }

        /// @brief Render current selection
        void doRender(R &render) const noexcept override {
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
        explicit Display(P &root, const T &val) :
            Widget{root}, value{val} {}

        explicit Display(const T &val) noexcept :
            value{val} {}

        /// @brief Render value with appropriate formatting
        void doRender(R &render) const noexcept override {
            render.value(value);
        }
    };

    /// @brief Widget wrapper adding label to another widget
    /// @tparam W Type of widget being labeled (must inherit from Widget)
    template<typename W> struct Labeled final : Widget {
        static_assert(std::is_base_of<Widget, W>::value, "W must be a Widget Subclass");

        using WrappedType = W;///< Type of wrapped widget implementation

    private:
        StringView label;///< Label text

    public:
        W wrapped;///< Wrapped widget instance

        explicit Labeled(P &root, StringView label, W impl) :
            Widget{root}, label{label}, wrapped{std::move(impl)} {}

        /// @brief Forward click event to wrapped widget
        /// @return Result from wrapped widget's onClick()
        [[nodiscard]] bool onClick() noexcept override { return wrapped.onClick(); }

        /// @brief Forward change event to wrapped widget
        /// @return Result from wrapped widget's onValue()
        [[nodiscard]] bool onValue(Ev value) noexcept override { return wrapped.onValue(value); }

        /// @brief Render label followed by wrapped widget
        void doRender(R &render) const noexcept override {
            render.value(label);
            render.colon();
            wrapped.doRender(render);
        }
    };

    /// @brief Spin box for adjusting numeric values with different modes
    /// @tparam T Numeric type for spin box value (must be arithmetic)
    template<typename T, StepMode M> struct SpinBox final : Widget, HasChangeHandler<T> {
        using Value = T;                   ///< Numeric value type
        static constexpr auto step_mode{M};///< Specialization step mode

    private:
        using StepAdjusterType = StepAdjuster<T>;
        using ValueAdjusterType = ValueAdjuster<T, M>;

        T value_;                        ///< Reference to value being controlled
        T step;                          ///< Current step size
        bool is_step_setting_mode{false};///< true when adjusting step size, false when adjusting value

    public:
        explicit SpinBox(
            T default_value = T{},
            T step = StepAdjusterType::default_step) noexcept :
            value_{default_value}, step{step} {}

        explicit SpinBox(
            P &root,
            T default_value = T{},
            T step = StepAdjusterType::default_step) :
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
        [[nodiscard]] bool onValue(Ev direction) noexcept override {
            if (is_step_setting_mode) {
                StepAdjusterType::adjust(step, direction);
            } else {
                ValueAdjusterType::adjust(value_, step, direction);
                HasChangeHandler<T>::invokeHandler(value_);
            }
            return true;// redraw required after adjustment
        }

        /// @brief Render current value or step size based on mode
        void doRender(R &render) const noexcept override {
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
}// namespace kf::ui::internal
