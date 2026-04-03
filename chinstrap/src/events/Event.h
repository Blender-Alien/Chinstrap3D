#pragma once

#include <string>
#include <optional>

namespace Chinstrap { struct Scene; }

namespace Chinstrap
{
    enum class EventType
    {
        WindowClose,
        WindowResize,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased,
        MouseMoved,
        MouseScrolled,
    };

    struct Event
    {
        // If you're wondering why plain 'int' types, it's because glfw provides the data as base types.
        typedef union
        {
            struct { int width; int height;} WindowResized;
            struct { int keyCode; bool repeat; } KeyPressed;
            struct { int keyCode; } KeyReleased;
            struct { int mouseButton; } MouseButtonPressed;
            struct { int mouseButton; } MouseButtonReleased;
            struct { int mouseX; int mouseY; } MouseMoved;
            struct { int mouseOffsetY; } MouseScrolled;
        } EventDataUnion;

        EventDataUnion eventData;
        EventType type;
        bool handled;

        // This is expensive, so should only be used for logging and such.
        [[nodiscard]] std::string ToString() const;

        explicit Event(const EventType type_arg, const EventDataUnion eventData_arg)
            : eventData(eventData_arg), type(type_arg), handled(false) {}
    };

    template<typename Context>
    void DispatchEvent(EventType type, Event& event,
        bool(*lambda)(const Event& event, std::optional<Context*> context),
        std::optional<Context*> context)
    {
        if (type == event.type)
        {
            if (lambda(event, context))
            {
                event.handled = true;
            }
        }
    }

    inline void DispatchEventNoContext(EventType type, Event& event, bool(*lambda)(const Event& event))
    {
        if (type == event.type)
        {
            if (lambda(event))
            {
                event.handled = true;
            }
        }
    }

    inline std::string Event::ToString() const
    {
        switch (type)
        {
        case EventType::WindowClose:
            return "Closing window";
        case EventType::WindowResize:
            return "Window resizing: " +
                std::to_string(eventData.WindowResized.height) + "x" +
                std::to_string(eventData.WindowResized.width);
        case EventType::KeyPressed:
            return "Key pressed: " +
                std::to_string(eventData.KeyPressed.keyCode) + " repeated: " +
                std::to_string(eventData.KeyPressed.repeat);
        case EventType::KeyReleased:
            return "Key released: " +
                std::to_string(eventData.KeyReleased.keyCode);
        case EventType::MouseButtonPressed:
            return "MouseButton pressed: " +
                std::to_string(eventData.MouseButtonPressed.mouseButton);
        case EventType::MouseButtonReleased:
            return "MouseButton released: " +
                std::to_string(eventData.MouseButtonReleased.mouseButton);
        case EventType::MouseMoved:
            return "Mouse moved: " +
                std::to_string(eventData.MouseMoved.mouseX) + "x" +
                std::to_string(eventData.MouseMoved.mouseY);
        case EventType::MouseScrolled:
            return "Mouse scrolled by: " +
                std::to_string(eventData.MouseScrolled.mouseOffsetY);
        default:
            return "INVALID EVENT";
        }
    }
}
