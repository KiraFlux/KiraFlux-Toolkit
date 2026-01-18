#pragma once

#include <kf/aliases.hpp>
#include <kf/attributes.hpp>

namespace kf {

/// @brief отдельное пространство имён для внешних компонентов UI
namespace ui {

/// @brief Система рендера
template<typename Impl> struct Render {
    friend Impl;

    // Управление

    /// @brief Подготовить буфер отрисовки
    void prepare() { impl().prepareImpl(); }

    /// @brief После рендера кадра
    void finish() { impl().finishImpl(); }

    /// @brief Начало отрисовки виджета
    void widgetBegin(usize index) { impl().widgetBeginImpl(index); }

    /// @brief Завершение отрисовки виджета
    void widgetEnd() { impl().widgetEndImpl(); }

    /// @brief Количество виджетов, которые ещё возможно отобразить
    kf_nodiscard usize widgetsAvailable() { return impl().widgetsAvailableImpl(); }

    // Значения

    /// @brief Заголовок страницы
    void title(const char *title) { impl().titleImpl(title); }

    /// @brief Отобразить строку
    void string(const char *str) { impl().stringImpl(str); }

    /// @brief Отобразить целое число
    void number(i32 integer) { impl().numberImpl(integer); }

    /// @brief Отобразить вещественное число
    void number(f64 real, u8 rounding) { impl().numberImpl(real, rounding); }

    // Оформление

    /// @brief Отобразить стрелку от края к виджету
    void arrow() { impl().arrowImpl(); }

    /// @brief Колонка (Разделитель)
    void colon() { impl().colonImpl(); }

    /// @brief Контрастный текст
    void contrastBegin() { impl().contrastBeginImpl(); }

    /// @brief Контрастный текст
    void contrastEnd() { impl().contrastEndImpl(); }

    /// @brief Блок
    void blockBegin() { impl().blockBeginImpl(); }

    /// @brief Блок
    void blockEnd() { impl().blockEndImpl(); }

    /// @brief Альтернативный блок
    void variableBegin() { impl().variableBeginImpl(); }

    /// @brief Альтернативный блок
    void variableEnd() { impl().variableEndImpl(); }

private:
    inline Impl &impl() { return *static_cast<Impl *>(this); }
};

}// namespace ui
}// namespace kf
