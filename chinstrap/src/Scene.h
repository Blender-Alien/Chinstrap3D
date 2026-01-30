#pragma once

#include "Event.h"

#include <memory>
#include <cassert>
#include <string>

#include "InputEvents.h"

namespace Chinstrap {
    
    struct Scene
    {
        virtual ~Scene() = default;

        virtual void OnBegin() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender() {}

        virtual void OnEvent(Event& event) {}
        virtual bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) {return false;}

        template<typename TScene>
        void QueueChangeToScene()
        {
            assert(queued == nullptr);
            queued = std::make_unique<TScene>();
        }
        std::unique_ptr<Scene> queued = nullptr;

        std::string GetName() const {return typeid(*this).name();};
    };

}
