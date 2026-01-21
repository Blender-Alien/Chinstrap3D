#pragma once

#include "Event.h"

namespace Chinstrap
{
    struct KeyPressedEvent : public Event
    {
        int keyCode;

        KeyPressedEvent(const int keyCode)
            : keyCode(keyCode) {}
        CHIN_EVENT_TYPE(KeyPressed)
    };
    struct KeyReleasedEvent : public Event
    {
        int keyCode;

        KeyReleasedEvent(const int keyCode)
            : keyCode(keyCode) {}
        CHIN_EVENT_TYPE(KeyReleased)
    };
}