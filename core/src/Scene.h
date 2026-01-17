#pragma once

namespace Chinstrap {
    
    class Scene
    {
    public:
        virtual ~Scene() = default;

        virtual void OnUpdate() {}
        virtual void OnRender() {}
    };

}
