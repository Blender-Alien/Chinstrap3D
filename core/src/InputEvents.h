#pragma once

#include "Event.h"

namespace Chinstrap
{
    struct KeyPressedEvent : public Event
    {
        int keyCode;

        std::string ToString() const override { return "Key pressed: " + std::to_string(keyCode); }

        explicit KeyPressedEvent(const int keyCode)
            : keyCode(keyCode) {}
        CHIN_EVENT_TYPE(KeyPressed)
    };
    struct KeyReleasedEvent : public Event
    {
        int keyCode;

        std::string ToString() const override { return "Key released: " + std::to_string(keyCode); }

        explicit KeyReleasedEvent(const int keyCode)
            : keyCode(keyCode) {}
        CHIN_EVENT_TYPE(KeyReleased)
    };

    struct MouseMovedEvent: public Event
    {
        double mouseX;
        double mouseY;

        std::string ToString() const override { return "Mouse moved: " + std::to_string(mouseX) + ", " + std::to_string(mouseY); }

        explicit MouseMovedEvent(const double x, const double y)
            : mouseX(x), mouseY(y) {}
        CHIN_EVENT_TYPE(MouseMoved)
    };
    struct MouseScrolledEvent: public Event
    {
        double mouseOffsetY;

        std::string ToString() const override { return "Mouse scrolled: " + std::to_string(mouseOffsetY); }

        explicit MouseScrolledEvent(const double y)
            : mouseOffsetY(y) {}
        CHIN_EVENT_TYPE(MouseScrolled)
    };
    struct MouseButtonPressedEvent : public Event
    {
        int button;

        std::string ToString() const override { return "MouseButton pressed: " + std::to_string(button); }

        explicit MouseButtonPressedEvent(const int button)
            : button(button) {}
        CHIN_EVENT_TYPE(MouseButtonPressed)
    };
    struct MouseButtonReleasedEvent : public Event
    {
        int button;

        std::string ToString() const override { return "MouseButton released: " + std::to_string(button); }

        explicit MouseButtonReleasedEvent(const int button)
            : button(button) {}
        CHIN_EVENT_TYPE(MouseButtonReleased)
    };
}