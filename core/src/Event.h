#pragma once

namespace Chinstrap
{
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
    };

#define CHIN_EVENT_TYPE(TYPE) virtual EventType GetEventType() const override { return EventType::TYPE; }

    struct Event
    {
        bool handled = false;
        virtual EventType GetEventType() const = 0;

        virtual ~Event() = default;
    };
}