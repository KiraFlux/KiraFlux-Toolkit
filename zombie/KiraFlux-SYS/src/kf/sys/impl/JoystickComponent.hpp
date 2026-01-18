#pragma once

#include <kf/String.hpp>

#include <kf/sys/abc/Component.hpp>

namespace kf::sys {

struct JoystickComponent final : kf::sys::Component {

    float x{0.0f};
    float y{0.0f};

    void display() override {
        constexpr auto text_offset = static_cast<Pixel>(3);
        constexpr auto format = "%+1.3f";

        const auto center_x = canvas.centerX();
        const auto center_y = canvas.centerY();

        const auto right_text_x = canvas.maxGlyphX() - text_offset;
        const auto text_offset_y = static_cast<Pixel>(center_y + text_offset);

        canvas.rect(0, 0, canvas.maxX(), canvas.maxY(), gfx::Canvas::Mode::FillBorder);

        canvas.line(
            center_x,
            center_y,
            static_cast<Pixel>(static_cast<float>(center_x) + x * static_cast<float>(center_x)),
            center_y);

        canvas.line(
            center_x,
            center_y,
            center_x,
            static_cast<Pixel>(static_cast<float>(center_y) - y * static_cast<float>(center_y)));

        ArrayString<8> buffer{};

        formatTo(buffer, format, x);
        canvas.setCursor(text_offset, text_offset);
        canvas.text(buffer.data());
        canvas.setCursor(static_cast<Pixel>(right_text_x), text_offset);
        canvas.text("X");

        formatTo(buffer, format, y);
        canvas.setCursor(text_offset, text_offset_y);
        canvas.text(buffer.data());
        canvas.setCursor(static_cast<Pixel>(right_text_x), text_offset_y);
        canvas.text("Y");
    }
};

}// namespace kf::sys
