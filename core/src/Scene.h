#pragma once

#include <memory>
#include <cassert>


namespace Chinstrap {
    
    struct Scene
    {
        virtual ~Scene() = default;

        virtual void OnUpdate() {}
        virtual void OnRender() {}

        template<typename TScene>
        void QueueChangeToScene()
        {
            assert(queued == nullptr);
            queued = std::make_unique<TScene>();
        }
        std::unique_ptr<Scene> queued = nullptr;
    };

}
