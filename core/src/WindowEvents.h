#pragma once

#include "Event.h"

namespace Chinstrap
{
    struct WindowClosedEvent : public Event
    {
        WindowClosedEvent() {}

        CHIN_EVENT_TYPE(WindowClose)
    };

    struct WindowResizedEvent : public Event
    {
        WindowResizedEvent(int width, int height)
            : width(width), height(height) {}

        int width;
        int height;

        CHIN_EVENT_TYPE(WindowResize)
    };
}