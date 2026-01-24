#pragma once

#include "Event.h"

#include <memory>
#include <cassert>
#include <string>


namespace Chinstrap {
    
    struct Scene
    {
        virtual ~Scene() = default;

        virtual void OnBegin() {}
        virtual void OnUpdate() {}
        virtual void OnRender() {}

        virtual void OnEvent(Event& event) {}

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
