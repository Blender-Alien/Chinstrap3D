#pragma once

#include "events/Event.h"
#include "events/InputEvents.h"

#include <memory>
#include <cassert>
#include <string>


namespace Chinstrap {
    
    struct Scene
    {
        std::function<std::unique_ptr<Scene>()> CreateQueued;
        float OnUpdateProfile = 0.0f;
        float OnRenderProfile = 0.0f;
        float OnEventProfile  = 0.0f;

        virtual ~Scene() = default;

        virtual void OnBegin() {}
        virtual void OnShutdown() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender() {}

        virtual void OnEvent(Event& event) {}
        virtual bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) {return false;}

        template<typename TScene>
        void QueueChangeToScene()
        {
            assert(CreateQueued == nullptr);
            CreateQueued = [](){ return std::move(std::make_unique<TScene>()); };
        }

        [[nodiscard]] virtual std::string GetName() const { assert(false); return ""; }
    };

}
