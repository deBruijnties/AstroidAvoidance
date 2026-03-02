#pragma once
#include <Core/engine.h>


class Game : public GalacticEngine::Core::Engine {
public:
    using Engine::Engine; // Inherit Engine constructor

protected:
    void OnStart() override;
    void OnUpdate() override;
    void OnProcessInput() override;
};
