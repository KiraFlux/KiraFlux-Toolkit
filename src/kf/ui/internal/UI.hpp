// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

// Std
#include <type_traits>
#include <utility>

// Lib
#include "kf/Function.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/ui/render/Tag.hpp"

// Internal
#include "kf/ui/internal/ComboBoxItem.hpp"
#include "kf/ui/internal/StepAdjuster.hpp"
#include "kf/ui/internal/StepMode.hpp"
#include "kf/ui/internal/ValueAdjuster.hpp"
#include "kf/ui/internal/ValueLimits.hpp"
#include "kf/ui/internal/ValuePlacement.hpp"

namespace kf::ui::internal {

/// @brief Internal UI Definitions
/// @tparam R Render System Implementation
/// @tparam V Event Value Type
/// @tparam P Page Type
template<typename R, typename V, typename P> struct UI final {
    kf_crtp_check(R, kf::ui::render::Tag);

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

        /// @brief Handle Event value
        /// @return true if redraw required, false otherwise
        [[nodiscard]] virtual bool onEventValue(V event_value) noexcept { return false; }

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
        StringView _label;///< Button label text

    public:
        ClickHandler on_click{nullptr};///< Click event handler

        explicit Button(P &root, StringView label) :
            Widget{root}, _label{label} {}

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
            render.value(_label);
            render.endBlock();
        }
    };

    /// @brief Checkbox widget for boolean input
    struct CheckBox final : Widget, HasChangeHandler<bool> {
    private:
        bool _state;

    public:
        explicit CheckBox(bool default_state = false) noexcept :
            _state{default_state} {}

        explicit CheckBox(P &root, bool default_state = false) :
            Widget{root}, _state{default_state} {}

        void state(bool new_state) noexcept {
            if (new_state != _state) {
                _state = new_state;
                HasChangeHandler<bool>::invokeHandler(_state);
            }
        }

        [[nodiscard]] bool state() const noexcept {
            return _state;
        }

        [[nodiscard]] bool onClick() noexcept override {
            state(not _state);
            return true;
        }

        [[nodiscard]] bool onEventValue(V event_value) noexcept override {
            state(event_value > 0);
            return true;
        }

        /// @brief Render checkbox with visual state indicator
        void doRender(R &render) const noexcept override {
            render.checkbox(_state);
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
        const ItemContainer _items;///< Available options
        isize _cursor{0};          ///< Current selection index

    public:
        explicit ComboBox(ItemContainer items) noexcept :
            _items{items} {}

        explicit ComboBox(P &root, ItemContainer items) :
            Widget{root}, _items{items} {}

        /// @brief Change selection based on direction
        /// @param value Navigation direction (positive/negative)
        [[nodiscard]] bool onEventValue(V event_value) noexcept override {
            moveCursor(event_value);
            HasChangeHandler<T>::invokeHandler(_items[_cursor].value());
            return true;// redraw required after selection change
        }

        /// @brief Render current selection
        void doRender(R &render) const noexcept override {
            render.beginAltBlock();
            render.value(_items[_cursor].key());
            render.endAltBlock();
        }

    private:
        /// @brief Move selection cursor with circular wrapping
        void moveCursor(isize delta) noexcept {
            _cursor = (_cursor + delta + N) % N;
        }
    };

    /// @brief Display widget for showing read-only values
    /// @tparam T Type of value to display
    template<typename T> struct Display final : Widget {
    private:
        T _value;///< Stored copy of the value

    public:
        explicit Display(P &root, const T &value) : Widget{root}, _value{value} {}
        explicit Display(const T &value) noexcept : _value{value} {}

        /// Update the displayed value
        void value(T new_value) noexcept { _value = new_value; }

        /// Get the current displayed value
        [[nodiscard]] T value() const noexcept { return _value; }

        void doRender(R &render) const noexcept override {
            render.value(_value);
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
        /// @return Result from wrapped widget's onEventValue()
        [[nodiscard]] bool onEventValue(V event_value) noexcept override { return wrapped.onEventValue(event_value); }

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

        T _value;///< Reference to value being controlled
        T _step; ///< Current step size

        bool is_step_setting_mode{false};///< true when adjusting step size, false when adjusting value

    public:
        explicit SpinBox(
            T default_value = T{},
            T step = StepAdjusterType::default_step) noexcept :
            _value{default_value}, _step{step} {}

        explicit SpinBox(
            P &root,
            T default_value = T{},
            T step = StepAdjusterType::default_step) :
            Widget{root}, _value{default_value}, _step{step} {}

        void value(T value) noexcept {
            _value = value;
            HasChangeHandler<T>::invokeHandler(_value);
        }

        [[nodiscard]] T value() const noexcept { return _value; }

        /// @brief Toggle between value adjustment and step adjustment modes
        /// @return true (redraw required after mode change)
        [[nodiscard]] bool onClick() noexcept override {
            is_step_setting_mode = not is_step_setting_mode;
            return true;
        }

        /// @brief Adjust value or step based on current mode
        /// @param event_value Adjustment scale
        [[nodiscard]] bool onEventValue(V event_value) noexcept override {
            if (is_step_setting_mode) {
                StepAdjusterType::adjust(_step, event_value);
            } else {
                ValueAdjusterType::adjust(_value, _step, event_value);
                HasChangeHandler<T>::invokeHandler(_value);
            }
            return true;// redraw required after adjustment
        }

        /// @brief Render current value or step size based on mode
        void doRender(R &render) const noexcept override {
            render.beginAltBlock();

            if (is_step_setting_mode) {
                render.arrow();
                render.value(_step);
            } else {
                render.value(_value);
            }

            render.endAltBlock();
        }
    };

    template<typename T> struct Slider final : Widget, HasChangeHandler<T> {
        using ValueType = T;

    private:
        T _value;
        T _step;

    public:
        T min_value{ValueLimits<T>::value_min};
        T max_value{ValueLimits<T>::value_max};
        bool show_value{false};
        ValuePlacement placement{ValuePlacement::Inside};

        explicit Slider(
            T default_value = T{},
            T step = StepAdjuster<T>::default_step) noexcept :
            _value{default_value}, _step{step} {}

        explicit Slider(
            P &root,
            T default_value = T{},
            T step = StepAdjuster<T>::default_step) :
            Widget{root}, _value{default_value}, _step{step} {}

        void value(T new_value) noexcept {
            new_value = kf::clamp(new_value, min_value, max_value);
            if (_value != new_value) {
                _value = new_value;
                HasChangeHandler<T>::invokeHandler(_value);
            }
        }

        [[nodiscard]] T value() const noexcept { return _value; }

        /// @brief Toggle slider numeric value display
        [[nodiscard]] bool onClick() noexcept override {
            show_value = not show_value;
            return true;// redraw required after mode change
        }

        /// @brief Adjust value
        /// @param event_value adjust change scale
        [[nodiscard]] bool onEventValue(V event_value) noexcept override {
            ValueAdjuster<T, StepMode::Arithmetic>::adjust(_value, _step, event_value);
            _value = kf::clamp(_value, min_value, max_value);
            HasChangeHandler<T>::invokeHandler(_value);
            return true;// redraw required after adjustment
        }

        void doRender(R &render) const noexcept override {
            render.slider(_value, min_value, max_value, show_value ? placement : ValuePlacement::Hidden);
        }
    };
};
}// namespace kf::ui::internal
