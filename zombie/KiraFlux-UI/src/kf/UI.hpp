#pragma once

#include <kf/algorithm.hpp>
#include <kf/attributes.hpp>
#include <kf/fn.hpp>
#include <kf/type_traits.hpp>
#include <kf/utility.hpp>

#include <kf/array.hpp>
#include <kf/queue.hpp>
#include <kf/vector.hpp>

#include <kf/aliases.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "kf/ui/Event.hpp"

namespace kf {

/// @brief Пользовательский интерфейс
/// @tparam R Реализация системы рендера (Наследник <code>kf::ui::Render</code>)
template<typename R> struct UI final : tools::Singleton<UI<R>> {
    friend tools::Singleton<UI<R>>;

    /// @brief Реализация системы рендера
    using RenderImpl = R;

    // alias для единообразия
    using Event = ui::Event;

    struct Page;

    /// @brief Виджет
    struct Widget {

        /// @brief Создать и добавить на страницу
        explicit Widget(Page &root) {
            root.addWidget(*this);
        }

        explicit Widget() = default;

        /// @brief Отрисовать виджет
        /// @param render Система отрисовки
        virtual void doRender(RenderImpl &render) const = 0;

        /// @brief Действие при событии клика
        /// @returns true - Нужна перерисовка
        /// @returns false - Перерисовка не требуется
        virtual bool onClick() { return false; }

        /// @brief Действие при изменении значения
        /// @param direction Величина изменения
        /// @returns true - Нужна перерисовка
        /// @returns false - Перерисовка не требуется
        virtual bool onChange(int direction) { return false; }

        /// @brief Внешняя отрисовка виджета
        /// @param render Система отрисовки
        /// @param focused Виджет в фокусе курсора
        void render(RenderImpl &render, bool focused) const {
            if (focused) {
                render.contrastBegin();
                doRender(render);
                render.contrastEnd();
            } else {
                doRender(render);
            }
        }
    };

    /// @brief Страница, содержит виджеты и обладает заголовком.
    struct Page {

    private:
        /// @brief Специальный виджет для создания кнопки перехода на страницу
        /// @note Не используется в пользовательском коде. Для связывания страниц используйте <code>kf::ui::Page::link</code>
        struct PageSetter final : Widget {

        private:
            /// @brief Страница перехода
            Page &target;

        public:
            explicit PageSetter(Page &target) :
                target{target} {}

            /// @brief Устанавливает активную страницу
            bool onClick() override {
                UI::instance().bindPage(target);
                return true;
            }

            void doRender(RenderImpl &render) const override {
                render.arrow();
                render.string(target.title);
            }
        };

        /// @brief Виджеты данной страницы
        vector<Widget *> widgets{};// todo Widget refs

        /// @brief Заголовок страницы.
        const char *title;

        /// @brief Курсор
        /// @details Индекс активного виджета
        usize cursor{0};

        /// @brief Виджет перехода к данной странице
        PageSetter to_this{*this};

    public:
        explicit Page(const char *title) :
            title{title} {}

        /// @brief Добавить виджет в данную страницу
        /// @param widget Добавляемый виджет
        void addWidget(Widget &widget) {
            widgets.push_back(&widget);
        }

        /// @brief Связывание страниц
        /// @details Добавляет виджеты перехода к страницам
        /// @param other Связываемая страница
        void link(Page &other) {
            this->addWidget(other.to_this);
            other.addWidget(this->to_this);
        }

        /// @brief Отобразить страницу
        /// @param render Система отрисовки
        void render(RenderImpl &render) {
            render.title(title);

            const auto available = render.widgetsAvailable();
            const auto start = (totalWidgets() > available) ? min(cursor, totalWidgets() - available) : 0;
            const auto end = min(start + available, totalWidgets());

            for (auto i = start; i < end; i += 1) {
                render.widgetBegin(i);
                widgets[i]->render(render, i == cursor);
                render.widgetEnd();
            }
        }

        /// @brief Отреагировать на событие
        /// @param event Входящее событие
        /// @return true Рендер требуется
        /// @return false Рендер не требуется
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

        /// @brief Общее кол-во виджетов
        kf_nodiscard inline usize totalWidgets() const { return static_cast<int>(widgets.size()); }

    private:
        /// @brief Максимальная позиция курсора
        kf_nodiscard inline usize cursorPositionMax() const { return totalWidgets() - 1; }

        /// @brief Сместить курсор на странице
        /// @param delta Величина смещения в индексах
        /// @return true Нужна перерисовка, Курсор установлен в новую позицию
        /// @return false Перерисовка не требуется, Курсор установлен не изменил позиции
        kf_nodiscard bool moveCursor(isize delta) {
            const auto last_cursor = cursor;
            cursor += delta;
            cursor = max(static_cast<isize>(cursor), 0);
            cursor = min(cursor, cursorPositionMax());
            return last_cursor != cursor;
        }
    };

private:
    /// @brief Входящие события
    queue<Event> events{};

    /// @brief Активная страница
    Page *active_page{nullptr};

    /// @brief Система отображения
    RenderImpl render_system{};

public:
    /// @brief Получить экземпляр настроек системы рендера
    typename RenderImpl::Settings &getRenderSettings() {
        return render_system.settings;
    }

    /// @brief Установить активную страницу
    void bindPage(Page &page) {
        active_page = &page;
    }

    /// @brief Добавить событие в очередь
    void addEvent(Event event) {
        events.push(event);
    }

    /// @brief Прокрутка входящих событий. Выполняет рендер при необходимости
    void poll() {
        // mostly time active page is not null, so null-check is after queue.
        if (events.empty() or nullptr == active_page) {
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

public:
    // built-in widgets

    /// @brief Кнопка - Виджет, реагирующий на клик
    struct Button final : Widget {

        /// @brief Обработчик клика
        using ClickHandler = fn<void()>;

    private:
        /// @brief Метка кнопки
        const char *label;

        /// @brief Внешний обработчик клика
        ClickHandler on_click;

    public:
        explicit Button(
            Page &root,
            const char *label,
            ClickHandler on_click) :
            Widget{root},
            label{label},
            on_click{move(on_click)} {}

        bool onClick() override {
            if (on_click) {
                on_click();
            }

            return false;
        }

        void doRender(RenderImpl &render) const override {
            render.blockBegin();
            render.string(label);
            render.blockEnd();
        }
    };

    /// @brief Чек-Бокс - Ввод булевого значения
    struct CheckBox final : Widget {

        /// @brief Тип внешнего обработчика изменения
        using ChangeHandler = fn<void(bool)>;

    private:
        /// @brief Обработчик изменения
        ChangeHandler on_change;

        /// @brief Состояние
        bool state;

    public:
        explicit CheckBox(
            ChangeHandler change_handler,
            bool default_state = false) :
            on_change{move(change_handler)},
            state{default_state} {}

        explicit CheckBox(
            Page &root,
            ChangeHandler change_handler,
            bool default_state = false) :
            Widget{root},
            on_change{move(change_handler)},
            state{default_state} {}

        bool onClick() override {
            setState(not state);
            return true;
        }

        bool onChange(int direction) override {
            setState(direction > 0);
            return true;
        }

        void doRender(RenderImpl &render) const override {
            render.string(state ? "[ 1 ]==" : "--[ 0 ]");
        }

    private:
        void setState(bool new_state) {
            state = new_state;

            if (on_change) {
                on_change(state);
            }
        }
    };

    /// @brief ComboBox - выбор из списка значений
    /// @tparam T Тип выбираемых значений
    /// @tparam N Кол-во выбираемых значений
    template<typename T, usize N> struct ComboBox final : Widget {
        static_assert(N >= 1, "N >= 1");

        /// @brief Тип выбираемого значения
        using Value = T;

        /// @brief Элемент выбора
        struct Item {

            /// @brief Наименование элемента
            const char *key;

            /// @brief Значение
            T value;
        };

        /// @brief Контейнер элементов
        using Container = kf::array<Item, N>;

    private:
        /// @brief Элементы выбора
        const Container items;

        /// @brief Изменяемое значение
        T &value;

        /// @brief Выбранное значение
        int cursor{0};

    public:
        explicit ComboBox(
            T &val,
            Container items) :
            items{move(items)},
            value{val} {}

        explicit ComboBox(
            Page &root,
            T &val,
            Container items) :
            Widget{root},
            items{move(items)},
            value{val} {}

        bool onChange(int direction) override {
            moveCursor(direction);

            value = items[cursor].value;

            return true;
        }

        void doRender(RenderImpl &render) const override {
            render.variableBegin();
            render.string(items[cursor].key);
            render.variableEnd();
        }

    private:
        /// @brief Сместить курсор
        /// @param delta смещение
        void moveCursor(int delta) {
            cursor += delta;
            cursor += N;
            cursor %= N;
        }
    };

    /// @brief Отображает значение
    template<typename T> struct Display final : Widget {

    private:
        /// @brief Отображаемое значение
        const T &value;

    public:
        explicit Display(
            Page &root,
            const T &val) :
            Widget{root},
            value{val} {}

        explicit Display(
            const T &val) :
            value{val} {}

        void doRender(RenderImpl &render) const override {
            kf_if_constexpr (kf::is_floating_point<T>::value) {
                render.number(static_cast<float>(value), 3);
            } else {
                render.number(value);
            }
        }
    };

    /// @brief Добавляет метку к виджету
    /// @tparam W Тип реализации виджета, к которому будет добавлена метка
    template<typename W> struct Labeled final : Widget {
        static_assert(kf::is_base_of<Widget, W>::value, "W must be a Widget Subclass");

        /// @brief Реализация виджета, к которому была добавлена метка
        using Impl = W;

    private:
        /// @brief Метка
        const char *label;

        /// @brief Виджет
        W impl;

    public:
        explicit Labeled(
            Page &root,
            const char *label,
            W impl) :
            Widget{root},
            label{label},
            impl{move(impl)} {}

        bool onClick() override { return impl.onClick(); }

        bool onChange(int direction) override { return impl.onChange(direction); }

        void doRender(RenderImpl &render) const override {
            render.string(label);
            render.colon();
            impl.doRender(render);
        }
    };

    /// @brief Спин-бокс - Виджет для изменения арифметического значения в указанном режиме
    template<typename T> struct SpinBox final : Widget {
        static_assert(kf::is_arithmetic<T>::value, "T must be arithmetic");

        /// @brief Тип скалярной величины виджета
        using Value = T;

        /// @brief Режим изменения значения
        enum class Mode : unsigned char {
            /// @brief Арифметическое изменение
            Arithmetic,

            /// @brief Арифметическое изменение (Только положительные)
            ArithmeticPositiveOnly,

            /// @brief Геометрическое изменение
            Geometric
        };

    private:
        /// @brief Флаг режима настройки шага
        bool is_step_setting_mode{false};

        /// @brief Режим изменения значения
        const Mode mode;

        /// @brief Изменяемое значение
        T &value;

        /// @brief Шаг изменения значения
        T step;

    public:
        explicit SpinBox(
            T &value,
            T step = static_cast<T>(1),
            Mode mode = Mode::Arithmetic) :
            mode{mode},
            value{value},
            step{step} {}

        explicit SpinBox(
            Page &root,
            T &value,
            T step = static_cast<T>(1),
            Mode mode = Mode::Arithmetic) :
            Widget{root},
            mode{mode},
            value{value},
            step{step} {}

        bool onClick() override {
            is_step_setting_mode = !is_step_setting_mode;
            return true;
        }

        bool onChange(int direction) override {
            if (is_step_setting_mode) {
                changeStep(direction);
            } else {
                changeValue(direction);
            }
            return true;
        }

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
        void displayNumber(RenderImpl &render, const T &number) const {
            kf_if_constexpr (kf::is_floating_point<T>::value) {
                render.number(static_cast<float>(number), 4);
            } else {
                render.number(number);
            }
        }

        /// @brief Изменить значение
        void changeValue(int direction) {
            if (mode == Mode::Geometric) {
                // Геометрическое изменение: умножаем/делим значение
                if (direction > 0) {
                    value *= step;
                } else {
                    value /= step;
                }
            } else {
                // Арифметическое изменение: прибавляем/вычитаем
                value += direction * step;

                // Проверка для режима только положительных значений
                if (mode == Mode::ArithmeticPositiveOnly and value < 0) {
                    value = 0;
                }
            }
        }

        /// @brief Изменить шаг
        void changeStep(int direction) {
            constexpr T step_multiplier{static_cast<T>(10)};

            if (direction > 0) {
                step *= step_multiplier;
            } else {
                step /= step_multiplier;

                // Защита от слишком маленьких шагов
                kf_if_constexpr (kf::is_integral<T>::value) {
                    if (step < 1) { step = 1; }
                }
            }
        }
    };
};

}// namespace kf
