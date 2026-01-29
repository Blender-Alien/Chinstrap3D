#pragma once

#include "Event.h"

namespace Chinstrap
{
    struct WindowClosedEvent : public Event
    {
        std::string ToString() const override { return "Closing window"; }

        explicit WindowClosedEvent() = default;
        CHIN_EVENT_TYPE(WindowClose)
    };

    struct WindowResizedEvent : public Event
    {
        int width;
        int height;

        std::string ToString() const override { return ("Changed Size: " + std::to_string(width) + "x" + std::to_string(height)); }

        explicit WindowResizedEvent(int width, int height)
            : width(width), height(height) {}
        CHIN_EVENT_TYPE(WindowResize)
    };
}