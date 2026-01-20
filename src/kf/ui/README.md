# KiraFlux-UI

*Минималистичный пользовательский интерфейс для встроенных систем на C++*

---

## Содержание
- [Архитектура](#архитектура)
- [Быстрый старт](#быстрый-старт)
- [Система рендера](#система-рендера)
- [События](#события)
- [Страницы и навигация](#страницы-и-навигация)
- [Виджеты](#виджеты)
- [Примеры использования](#примеры-использования)
- [Arduino-пример](#arduino-пример)
- [API Reference](#api-reference)

---

## Архитектура

KiraFlux-UI построена на принципах статического полиморфизма и шаблонного программирования:

- **Статический полиморфизм** через CRTP (Curiously Recurring Template Pattern) для системы рендера
- **Динамический полиморфизм** для виджетов - оптимальное решение для пользовательских расширений
- **Шаблонный UI** с подставляемой реализацией рендера
- **Минимальные накладные расходы**
- **Header-only** библиотека

## Быстрый старт

```cpp
#include <kf/UI.hpp>
#include <kf/ui/TextRender.hpp>

// Определяем UI с текстовым рендером
using TextUI = kf::UI<kf::ui::TextRender>;

// Создаем страницу
TextUI::Page main_page("Main Menu");

// Добавляем виджеты
float value = 12.34f;

TextUI::Button button(main_page, "Click me", []() {
    // Обработчик клика
});

TextUI::SpinBox<float> spinner(main_page, value, 0.1f);

// Устанавливаем активную страницу
TextUI::instance().bindPage(main_page);
```

## Система рендера

### Базовая структура Render

```cpp
template<typename Impl> 
struct Render {
    // Управление
    void prepare();
    void finish();
    void beginWidget(usize index);
    void endWidget();
    
    // Значения
    void string(const char *str);
    void number(i32 integer);
    void number(f64 real, u8 rounding);
    
    // Оформление
    void arrow();
    void colon();
    void beginContrast();
    void endContrast();
    void beginBlock();
    void endBlock();
    void beginAltBlock();
    void endAltBlock();
};
```

### Создание пользовательского рендера

```cpp
struct MyCustomRender : kf::ui::Render<MyCustomRender> {
    // Дружественный класс для доступа к приватным методам
    friend struct kf::ui::Render<MyCustomRender>;
    
    struct Settings {
        // Настройки вашего рендера
    } settings;

private:
    // Реализация методов (скрыта от пользователя)
    void prepareImpl() { /* ... */ }
    void finishImpl() { /* ... */ }
    void stringImpl(const char *str) { /* ... */ }
    // ... остальные методы
};

// Использование пользовательского рендера
using MyUI = kf::UI<MyCustomRender>;
```

### Готовая реализация: TextRender

```cpp
#include <kf/ui/TextRender.hpp>

using TextUI = kf::UI<kf::ui::TextRender>;

// Настройка текстового рендера
auto& render_settings = TextUI::instance().renderSettings();
render_settings.buffer = text_buffer;
render_settings.rows_total = 4;
render_settings.row_max_length = 20;
render_settings.on_render_finish = [](auto str) {
    // Обработка отрендеренного текста
};
```

## События

### Типы событий

```cpp
// Внутри TextUI::Event
enum class Type {
    None,               // Пустое событие
    Update,             // Принудительный рендер
    PageCursorMove,     // Перемещение курсора (+1/-1)
    WidgetClick,        // Клик на виджете
    WidgetValueChange   // Изменение значения (+1/-1)
};
```

### Отправка событий

```cpp
auto& ui = TextUI::instance();

// Перемещение курсора вниз
ui.addEvent(TextUI::Event::PageCursorMove(+1));

// Клик на активном виджете
ui.addEvent(TextUI::Event::WidgetClick());

// Принудительная перерисовка
ui.addEvent(TextUI::Event::Update());
```

## Страницы и навигация

### Создание страниц

```cpp
TextUI::Page page1("Settings");
TextUI::Page page2("Data");

// Связывание страниц (автоматически создает кнопки перехода)
page1.link(page2);
```

### Навигация между страницами

```cpp
// Установка активной страницы
TextUI::instance().bindPage(page1);
```

## Виджеты

### Button - Кнопка

```cpp
TextUI::Button button(
    page, 
    "Apply", 
    []() {
        // Действие при клике
        applySettings();
    }
);
```

### CheckBox - Чекбокс

```cpp
bool state = false;

TextUI::CheckBox checkbox(
    page,
    [](bool new_state) {
        // Обработчик изменения
        state = new_state;
    },
    false // начальное состояние
);
```

### SpinBox - Числовой ввод

```cpp
float value = 0.0f;

TextUI::SpinBox<float> spinbox(
    page,
    value,           // связываем с переменной
    0.1f,            // шаг изменения
    TextUI::SpinBox<float>::Mode::Arithmetic
);
```

### ComboBox - Выбор из списка

```cpp
int selected_value = 0;

TextUI::ComboBox<int, 3> combobox(
    page,
    selected_value,
    {{
        {"Option 1", 1},
        {"Option 2", 2}, 
        {"Option 3", 3}
    }}
);
```

### Display - Отображение значения

```cpp
float sensor_value = 25.5f;

TextUI::Display<float> display(
    page,
    sensor_value
);
```

### Labeled - Виджет с меткой

```cpp
// Правильное использование Labeled
using TemperatureDisplay = TextUI::Labeled<TextUI::Display<float>>;

TemperatureDisplay temp_display(
    page,
    "Temperature",
    TextUI::Display<float>{sensor_value}  // Внутренний виджет
);
```

## Примеры использования

### Простой интерфейс настроек

```cpp
TextUI::Page settings("Settings");

float brightness = 0.8f;
int contrast = 50;
bool enabled = true;

// Используем Labeled с SpinBox
using BrightnessControl = TextUI::Labeled<TextUI::SpinBox<float>>;

BrightnessControl brightness_ctl(
    settings,
    "Brightness", 
    TextUI::SpinBox<float>{brightness, 0.1f}
);

// Используем Labeled с CheckBox  
using EnableControl = TextUI::Labeled<TextUI::CheckBox>;

EnableControl enable_ctl(
    settings,
    "Enabled",
    TextUI::CheckBox{[](bool state) { enabled = state; }}
);

TextUI::Button apply_btn(
    settings,
    "Apply",
    applySettings
);
```

### Интерфейс данных с несколькими страницами

```cpp
TextUI::Page data_page("Data View");
TextUI::Page config_page("Configuration");

// Связываем страницы
data_page.link(config_page);

// Страница данных
float temp = 25.0f;
float humidity = 60.0f;

// Используем Labeled для дисплеев
using TempDisplay = TextUI::Labeled<TextUI::Display<float>>;
using HumidityDisplay = TextUI::Labeled<TextUI::Display<float>>;

TempDisplay temp_display(data_page, "Temperature", 
    TextUI::Display<float>{temp});
HumidityDisplay humidity_display(data_page, "Humidity",
    TextUI::Display<float>{humidity});

// Страница конфигурации
using TempCalib = TextUI::Labeled<TextUI::SpinBox<float>>;
using HumCalib = TextUI::Labeled<TextUI::SpinBox<float>>;

TempCalib temp_calib(config_page, "Temp Cal", 
    TextUI::SpinBox<float>{temp_calibration, 0.1f});
HumCalib hum_calib(config_page, "Hum Cal",
    TextUI::SpinBox<float>{hum_calibration, 0.1f});

// Устанавливаем начальную страницу
TextUI::instance().bindPage(data_page);
```

## Arduino-пример

```cpp
#include <kf/UI.hpp>
#include <kf/ui/TextRender.hpp>

// Определяем UI с текстовым рендером
using TextUI = kf::UI<kf::ui::TextRender>;

// Буфер для текстового вывода
char text_buffer[250];

// Создаем страницу
TextUI::Page main_page("Main Menu");

// Определяем типы для виджетов с метками
using TempDisplay = TextUI::Labeled<TextUI::Display<float>>;
using ModeSelector = TextUI::Labeled<TextUI::ComboBox<int, 3>>;

// Данные приложения
float sensor_value = 25.5f;
int selected_mode = 0;

// Создаем виджеты
TempDisplay temp_display(
    main_page,
    "Temperature",
    TempDisplay::Impl{sensor_value}
);

ModeSelector mode_selector(
    main_page,
    "Mode",
    ModeSelector::Impl{
        selected_mode,
        {{
            {"Auto", 0},
            {"Manual", 1},
            {"Test", 2}
        }}
    }
);

TextUI::Button calibrate_btn(
    main_page,
    "Calibrate",
    []() {
        // Калибровка датчика
        calibrateSensor();
    }
);

void setup() {
    Serial.begin(115200);
    
    auto& ui = TextUI::instance();
    
    // Настройка рендера
    auto& render_settings = ui.renderSettings();
    render_settings.buffer = {reinterpret_cast<uint8_t*>(text_buffer), 
                             sizeof(text_buffer)};
    render_settings.rows_total = 4;
    render_settings.row_max_length = 20;
    render_settings.on_render_finish = [](const auto& str) {
        Serial.print("UI: ");
        Serial.println(reinterpret_cast<const char*>(str.data()));
    };

    // Устанавливаем активную страницу
    ui.bindPage(main_page);
    
    // Первоначальная отрисовка
    ui.addEvent(TextUI::Event{TextUI::Event::Type::Update});
}

void loop() {
    // Обработка событий UI
    TextUI::instance().poll();
    
    // Здесь может быть обработка аппаратных кнопок
    // и отправка событий в UI
    delay(100);
}

// Пример обработки кнопок
void handleUpButton() {
    TextUI::instance().addEvent(
        TextUI::Event{TextUI::Event::Type::PageCursorMove, -1}
    );
}

void handleDownButton() {
    TextUI::instance().addEvent(
        TextUI::Event{TextUI::Event::Type::PageCursorMove, +1}
    );
}

void handleSelectButton() {
    TextUI::instance().addEvent(
        TextUI::Event{TextUI::Event::Type::WidgetClick}
    );
}
```

### Расширенный пример с ComboBox

```cpp
#include <kf/UI.hpp>
#include <kf/ui/TextRender.hpp>

using TextUI = kf::UI<kf::ui::TextRender>;

// Создаем страницу с различными типами ComboBox
TextUI::Page settings_page("Device Settings");

// ComboBox для выбора режима работы
using ModeSelector = TextUI::Labeled<TextUI::ComboBox<int, 3>>;

int work_mode = 0;
ModeSelector mode_selector(
    settings_page,
    "Work Mode",
    TextUI::ComboBox<int, 3>{
        work_mode,
        {{
            {"Standard", 0},
            {"Eco", 1},
            {"Turbo", 2}
        }}
    }
);

// ComboBox для выбора единиц измерения
using UnitSelector = TextUI::Labeled<TextUI::ComboBox<int, 2>>;

int temperature_units = 0;
UnitSelector unit_selector(
    settings_page,
    "Units",
    TextUI::ComboBox<int, 2>{
        temperature_units,
        {{
            {"Celsius", 0},
            {"Fahrenheit", 1}
        }}
    }
);

// ComboBox для выбора интервала обновления
using IntervalSelector = TextUI::Labeled<TextUI::ComboBox<int, 4>>;

int update_interval = 1;
IntervalSelector interval_selector(
    settings_page,
    "Interval",
    TextUI::ComboBox<int, 4>{
        update_interval,
        {{
            {"1 sec", 1},
            {"5 sec", 5},
            {"30 sec", 30},
            {"1 min", 60}
        }}
    }
);

void setup() {
    auto& ui = TextUI::instance();
    // ... настройка рендера
    ui.bindPage(settings_page);
}
```

## API Reference

### UI\<RenderImpl>

```cpp
template<typename RenderImpl>
struct UI {
    // Управление
    void bindPage(Page& page);
    void addEvent(Event event);
    void poll();
    
    // Настройки
    auto& renderSettings();
    
    // Виджеты
    struct Button;
    struct CheckBox; 
    struct ComboBox;
    struct Display;
    template<typename W> struct Labeled;
    struct SpinBox;
    
    struct Page;
    struct Widget;
};
```

### Page

```cpp
struct Page {
    explicit Page(const char* title);
    void addWidget(Widget& widget);
    void link(Page& other);
};
```

### Event

```cpp
struct Event {
    enum class Type { /* ... */ };
    using Value = i8;
    
    explicit Event(Type type, Value value = 0);
    Type type() const;
    Value value() const;
};
```

### Labeled\<W>

```cpp
template<typename W>
struct Labeled : Widget {
    using Impl = W;  // Тип внутреннего виджета
    
    explicit Labeled(
        Page& root,
        const char* label,
        W impl
    );
};
```

---

## Особенности работы

### Производительность
- **Zero-cost абстракции** благодаря статическому полиморфизму в системе рендера*
- **Оптимальный полиморфизм** для виджетов - динамический, где это уместно
- **Отсутствие динамической памяти** (кроме std::function в обработчиках)
- **Минимальные накладные расходы** на обработку событий

### Расширяемость
- **Легко создать кастомный рендер** для любого способа отображения (текст, графика...)
- **Гибкая система виджетов** с возможностью наследования
- **Поддержка любых типов данных** через шаблоны

### Безопасность
- **Проверки времени компиляции** через static_assert
- **Type-safe работа с событиями**
- **Инкапсуляция реализации** рендера

**Лицензия: MIT** ([LICENSE](./LICENSE))

---

*KiraFlux-UI - минималистичный, но мощный фреймворк для создания пользовательских интерфейсов в embedded системах с минимальными требованиями к ресурсам.*