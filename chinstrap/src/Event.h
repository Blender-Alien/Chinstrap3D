#pragma once

#include <string>
#include <functional>

namespace Chinstrap
{
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
    };

#define CHIN_EVENT_TYPE(TYPE) virtual EventType GetEventType() const override { return EventType::TYPE; }\
                              static EventType GetStaticEventType() { return EventType::TYPE; }

    struct Event
    {
        bool handled = false;

        virtual EventType GetEventType() const = 0;
        virtual std::string ToString() const = 0;

        virtual ~Event() = default;
    };

    struct EventDispatcher
    {
        template <typename T>
        static void Dispatch(Event &event, const std::function<bool(T &dispatchedEvent)> &func)
        {
            if (event.GetEventType() == T::GetStaticEventType() )
            {
                event.handled = func(static_cast<T&>(event));
            }
        }
    };
}
