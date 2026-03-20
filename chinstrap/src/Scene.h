#pragma once

#include "events/Event.h"
#include "events/InputEvents.h"

#include "Application.h"

namespace Chinstrap::Application {struct App;}

namespace Chinstrap::Resourcer {struct ResourceManager;}

namespace Chinstrap {
    
    struct Scene
    {
        std::function<std::unique_ptr<Scene>(Resourcer::ResourceManager* pResourceManager)> CreateQueued;

        Resourcer::ResourceManager* const pResourceManager;

        // Points to the first element in memory
        VkCommandBuffer* standardCmdBufferArray = nullptr;
        VkCommandPool* standardCmdPool = nullptr;

        float OnUpdateProfile = 0.0f;
        float OnRenderProfile = 0.0f;
        float OnEventProfile  = 0.0f;

        explicit Scene(Resourcer::ResourceManager* pResourceManger_arg)
            :  pResourceManager(pResourceManger_arg) {}

        virtual ~Scene() = default;

        virtual void OnBegin() {}
        virtual void OnShutdown() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender(uint32_t currentFrame) {}

        virtual void OnEvent(Event& event) {}
        virtual bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) {return false;}

        template<typename TScene>
        void QueueChangeToScene()
        {
            assert(CreateQueued == nullptr);
            CreateQueued = [](Resourcer::ResourceManager* pResourceManager)
            {
                return std::move(std::make_unique<TScene>(pResourceManager));
            };
        }

        [[nodiscard]] virtual std::string GetName() const { assert(false); return ""; }
    };

}
